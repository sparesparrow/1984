#include "HandInteraction.h"
#include "IGrabbable.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HapticFeedbackEffect_Curve.h"
#include "Project1984/Core/Characters/WinstonCharacter.h"
#include "Engine/World.h"

UHandInteraction::UHandInteraction()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrabState        = EGrabState::Empty;
	HeldActor        = nullptr;
	HoverActor       = nullptr;
	GrabRadius       = 15.0f;
	ExamineDistance  = 30.0f;
	GrabSocketName   = FName("GrabSocket");
	VRCamera         = nullptr;
	PrevHandLocation = FVector::ZeroVector;

	VelocityHistory.Init(FVector::ZeroVector, VelocityHistorySize);
}

void UHandInteraction::BeginPlay()
{
	Super::BeginPlay();

	PrevHandLocation = GetOwner()->GetActorLocation();

	// Cache VR camera from the owning pawn.
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		VRCamera = Pawn->FindComponentByClass<UCameraComponent>();
	}

	UE_LOG(LogTemp, Log, TEXT("HandInteraction: Initialized on '%s'. GrabRadius=%.0f cm."),
		GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), GrabRadius);
}

void UHandInteraction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Track hand velocity for throw calculation.
	const FVector CurrentLoc = GetOwner()->GetActorLocation();
	const FVector Velocity    = (DeltaTime > KINDA_SMALL_NUMBER)
		? (CurrentLoc - PrevHandLocation) / DeltaTime
		: FVector::ZeroVector;
	PrevHandLocation = CurrentLoc;

	// Shift ring buffer and store latest velocity.
	for (int32 i = VelocityHistorySize - 1; i > 0; --i)
	{
		VelocityHistory[i] = VelocityHistory[i - 1];
	}
	VelocityHistory[0] = Velocity;

	if (GrabState == EGrabState::Empty)
	{
		AActor* Nearest = FindNearestGrabbable();
		if (Nearest != HoverActor)
		{
			HoverActor = Nearest;
		}
		GrabState = HoverActor ? EGrabState::Hovering : EGrabState::Empty;
	}
	else if (GrabState == EGrabState::Hovering)
	{
		if (!FindNearestGrabbable())
		{
			GrabState  = EGrabState::Empty;
			HoverActor = nullptr;
		}
	}
	else if (GrabState == EGrabState::Grabbing && HeldActor)
	{
		// Transition to Examining when the held object is brought close to the camera/face.
		if (VRCamera)
		{
			const float DistToCamera = FVector::Dist(
				HeldActor->GetActorLocation(), VRCamera->GetComponentLocation());
			if (DistToCamera < ExamineDistance)
			{
				GrabState = EGrabState::Examining;
				UE_LOG(LogTemp, Verbose, TEXT("HandInteraction: Examining '%s'."),
					*HeldActor->GetName());
			}
		}
	}
	else if (GrabState == EGrabState::Examining && HeldActor)
	{
		// Return to Grabbing if the object is moved away from the face.
		if (VRCamera)
		{
			const float DistToCamera = FVector::Dist(
				HeldActor->GetActorLocation(), VRCamera->GetComponentLocation());
			if (DistToCamera >= ExamineDistance * 1.5f)
			{
				GrabState = EGrabState::Grabbing;
			}
		}
	}
}

void UHandInteraction::TryGrab()
{
	if (GrabState == EGrabState::Empty || GrabState == EGrabState::Hovering)
	{
		AActor* Target = HoverActor ? HoverActor : FindNearestGrabbable();
		if (Target)
		{
			AttachToHand(Target);
			GrabState  = EGrabState::Grabbing;
			HoverActor = nullptr;

			// Notify WinstonCharacter if the diary was grabbed.
			if (AWinstonCharacter* Winston = Cast<AWinstonCharacter>(
					GetWorld()->GetFirstPlayerController()
						? GetWorld()->GetFirstPlayerController()->GetPawn()
						: nullptr))
			{
				if (Target->ActorHasTag(FName("Diary")))
				{
					Winston->bHasDiary = true;
					UE_LOG(LogTemp, Log, TEXT("HandInteraction: Winston picked up the diary."));
				}
			}

			// Haptic pulse: short, sharp feedback on successful grab.
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				const bool bIsLeft = GetOwner() && GetOwner()->GetName().Contains(TEXT("Left"));
				PC->PlayDynamicForceFeedback(
					/*Intensity=*/0.6f,
					/*Duration=*/0.08f,
					/*bAffectsLeftLarge=*/bIsLeft,
					/*bAffectsLeftSmall=*/bIsLeft,
					/*bAffectsRightLarge=*/!bIsLeft,
					/*bAffectsRightSmall=*/!bIsLeft);
			}
		}
	}
}

void UHandInteraction::Release()
{
	if (HeldActor)
	{
		// Compute throw velocity before detaching.
		const FVector ThrowVelocity = ComputeThrowVelocity();

		// Notify the grabbable object that it's being let go.
		if (HeldActor->Implements<UGrabbable>())
		{
			IGrabbable::Execute_OnReleased(HeldActor, GetOwner());
		}

		DetachFromHand();
		GrabState = EGrabState::Empty;

		// Apply throw impulse if the hand was moving.
		if (!ThrowVelocity.IsNearlyZero(10.0f))
		{
			if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(HeldActor ? HeldActor->GetRootComponent() : nullptr))
			{
				if (Root->IsSimulatingPhysics())
				{
					Root->AddImpulse(ThrowVelocity * Root->GetMass(), NAME_None, /*bVelChange=*/false);
				}
			}
		}

		// Soft haptic fade-out on release.
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			const bool bIsLeft = GetOwner() && GetOwner()->GetName().Contains(TEXT("Left"));
			PC->PlayDynamicForceFeedback(
				/*Intensity=*/0.2f,
				/*Duration=*/0.04f,
				/*bAffectsLeftLarge=*/bIsLeft,
				/*bAffectsLeftSmall=*/bIsLeft,
				/*bAffectsRightLarge=*/!bIsLeft,
				/*bAffectsRightSmall=*/!bIsLeft);
		}
	}
}

bool UHandInteraction::IsHolding() const
{
	return HeldActor != nullptr;
}

void UHandInteraction::NotifyHateParticipation()
{
	// Find any active telescreen to notify about hate participation.
	for (TObjectIterator<class UTelescreenComponent> It; It; ++It)
	{
		if (It->GetWorld() == GetWorld() &&
			It->State == ETelescreenState::TwoMinutesHate)
		{
			It->bParticipatedInHate = true;
		}
	}
}

AActor* UHandInteraction::FindNearestGrabbable() const
{
	if (!GetOwner() || !GetWorld()) return nullptr;

	const FVector HandLocation = GetOwner()->GetActorLocation();

	// Sphere overlap centred on the hand's root position.
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HandInteractionOverlap), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		HandLocation,
		FQuat::Identity,
		ECC_PhysicsBody,
		FCollisionShape::MakeSphere(GrabRadius),
		Params);

	AActor* Best     = nullptr;
	float   BestDist = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == GetOwner()) continue;

		// Accept both tagged actors and those implementing the IGrabbable interface.
		const bool bTagged     = Candidate->ActorHasTag(FName("Grabbable"));
		const bool bInterface  = Candidate->Implements<UGrabbable>();
		if (!bTagged && !bInterface) continue;

		// If implementing IGrabbable, check if it can be grabbed right now.
		if (bInterface && !IGrabbable::Execute_CanBeGrabbed(Candidate)) continue;

		const float Dist = FVector::Dist(HandLocation, Candidate->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Best     = Candidate;
		}
	}

	return Best;
}

void UHandInteraction::AttachToHand(AActor* Target)
{
	if (!Target || !GetOwner()) return;

	HeldActor = Target;

	// Disable physics so the object follows the hand exactly.
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		Root->SetSimulatePhysics(false);
		Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Snap to hand actor location, then attach.
	Target->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// Notify the object that it's been grabbed.
	if (Target->Implements<UGrabbable>())
	{
		IGrabbable::Execute_OnGrabbed(Target, GetOwner());
	}

	UE_LOG(LogTemp, Log, TEXT("HandInteraction: Grabbed '%s'."), *Target->GetName());
}

void UHandInteraction::DetachFromHand()
{
	if (!HeldActor) return;

	// Re-enable collision and physics.
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(HeldActor->GetRootComponent()))
	{
		Root->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Root->SetSimulatePhysics(true);
	}

	HeldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	UE_LOG(LogTemp, Log, TEXT("HandInteraction: Released '%s'."), *HeldActor->GetName());
	HeldActor = nullptr;
}

FVector UHandInteraction::ComputeThrowVelocity() const
{
	// Average the velocity history, weighted toward the most recent frames.
	FVector Total  = FVector::ZeroVector;
	float   Weight = 0.0f;
	for (int32 i = 0; i < VelocityHistorySize; ++i)
	{
		const float W = FMath::Pow(0.7f, static_cast<float>(i));
		Total  += VelocityHistory[i] * W;
		Weight += W;
	}
	return (Weight > KINDA_SMALL_NUMBER) ? Total / Weight : FVector::ZeroVector;
}
