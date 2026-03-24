#include "HandInteraction.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "Project1984/Core/Characters/WinstonCharacter.h"

UHandInteraction::UHandInteraction()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrabState      = EGrabState::Empty;
	HeldActor      = nullptr;
	HoverActor     = nullptr;
	GrabRadius     = 15.0f;
	ExamineDistance = 30.0f;
	GrabSocketName = FName("GrabSocket");
	VRCamera       = nullptr;
	PrevHandLocation = FVector::ZeroVector;

	VelocityHistory.Init(FVector::ZeroVector, VelocityHistorySize);
}

void UHandInteraction::BeginPlay()
{
	Super::BeginPlay();

	PrevHandLocation = GetOwner()->GetActorLocation();

	// Cache VR camera from the owning pawn
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

	// Track hand velocity for throw calculation
	const FVector CurrentLoc = GetOwner()->GetActorLocation();
	const FVector Velocity    = (DeltaTime > KINDA_SMALL_NUMBER)
		? (CurrentLoc - PrevHandLocation) / DeltaTime
		: FVector::ZeroVector;
	PrevHandLocation = CurrentLoc;

	// Shift ring buffer and store latest velocity
	for (int32 i = VelocityHistorySize - 1; i > 0; --i)
	{
		VelocityHistory[i] = VelocityHistory[i - 1];
	}
	VelocityHistory[0] = Velocity;

	if (GrabState == EGrabState::Empty)
	{
		// Find nearest grabbable and enter Hovering state
		AActor* Nearest = FindNearestGrabbable();
		if (Nearest != HoverActor)
		{
			HoverActor = Nearest;
		}
		GrabState = HoverActor ? EGrabState::Hovering : EGrabState::Empty;
	}
	else if (GrabState == EGrabState::Hovering)
	{
		// Remain in Hovering as long as an object is nearby; drop back to Empty if not
		if (!FindNearestGrabbable())
		{
			GrabState  = EGrabState::Empty;
			HoverActor = nullptr;
		}
	}
	else if (GrabState == EGrabState::Grabbing && HeldActor)
	{
		// Transition to Examining when the held object is brought close to the camera/face
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
		// Return to Grabbing if the object is moved away from the face
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

			// Notify WinstonCharacter if the diary was grabbed
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

			// Haptic feedback — Blueprint-callable via PlayerController
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->PlayHapticEffect(nullptr, EControllerHand::Right, 0.5f);
			}
		}
	}
}

void UHandInteraction::Release()
{
	if (HeldActor)
	{
		// Compute throw velocity before detaching
		const FVector ThrowVelocity = ComputeThrowVelocity();

		DetachFromHand();
		GrabState = EGrabState::Empty;

		// Apply throw impulse if the hand was moving
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

		// Release haptic feedback
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->PlayHapticEffect(nullptr, EControllerHand::Right, 0.3f);
		}
	}
}

bool UHandInteraction::IsHolding() const
{
	return HeldActor != nullptr;
}

void UHandInteraction::NotifyHateParticipation()
{
	// Find any active telescreen to notify about hate participation
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
	if (!GetOwner())
	{
		return nullptr;
	}

	const FVector HandLocation = GetOwner()->GetActorLocation();

	// Sphere overlap to find candidate actors
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		HandLocation,
		FQuat::Identity,
		ECC_PhysicsBody,
		FCollisionShape::MakeSphere(GrabRadius),
		Params);

	AActor* Best       = nullptr;
	float   BestDist   = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == GetOwner())
		{
			continue;
		}

		// Must be tagged as Grabbable
		if (!Candidate->ActorHasTag(FName("Grabbable")))
		{
			continue;
		}

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
	if (!Target)
	{
		return;
	}

	HeldActor = Target;

	// Disable physics on the grabbed object so it follows the hand exactly
	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		Root->SetSimulatePhysics(false);
		Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Attach to the owner (VR hand) at the designated socket
	Target->AttachToActor(GetOwner(),
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld, /*bWeldSimulatedBodies=*/true));

	UE_LOG(LogTemp, Log, TEXT("HandInteraction: Grabbed '%s'."), *Target->GetName());
}

void UHandInteraction::DetachFromHand()
{
	if (!HeldActor)
	{
		return;
	}

	// Re-enable collision and physics
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
	// Average the velocity history, weighted toward the most recent frames
	FVector Total  = FVector::ZeroVector;
	float   Weight = 0.0f;
	for (int32 i = 0; i < VelocityHistorySize; ++i)
	{
		const float W = FMath::Pow(0.7f, static_cast<float>(i)); // decay weight
		Total  += VelocityHistory[i] * W;
		Weight += W;
	}
	return (Weight > KINDA_SMALL_NUMBER) ? Total / Weight : FVector::ZeroVector;
}
