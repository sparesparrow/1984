#include "SuspicionComponent.h"
#include "Kismet/GameplayStatics.h"

USuspicionComponent::USuspicionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	PersonalSuspicion = 0.0f;
	SuspicionDecayRate = 0.01f; // Slow natural decay per second
	bUnderSurveillance = false;
}

void USuspicionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register the owning actor with the global SurveillanceSystem
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
		{
			SS->RegisterSurveillanceActor(GetOwner());
			UE_LOG(LogTemp, Log, TEXT("SuspicionComponent: Registered '%s' with SurveillanceSystem."),
				GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
		}
	}
}

void USuspicionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplySuspicionDecay(DeltaTime);

	// Check whether the owner is within surveillance coverage.
	// We do a lightweight overlap query around the owner; any registered
	// surveillance actor (telescreen, NPC) that overlaps the radius counts
	// as active surveillance.
	if (GetOwner() && GetWorld())
	{
		const FVector MyLoc = GetOwner()->GetActorLocation();

		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		const bool bAnyOverlap = GetWorld()->OverlapMultiByChannel(
			Overlaps,
			MyLoc,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(800.0f), // 8-metre surveillance radius
			Params);

		bool bWatched = false;
		if (bAnyOverlap)
		{
			UGameInstance* GI = GetWorld()->GetGameInstance();
			USurveillanceSystem* SS = GI ? GI->GetSubsystem<USurveillanceSystem>() : nullptr;

			for (const FOverlapResult& Result : Overlaps)
			{
				AActor* Other = Result.GetActor();
				if (SS && Other && SS->GetActiveSurveillanceCount() > 0)
				{
					// Check if this overlapping actor is a registered surveillance source
					// (The ISurveillanceSource interface check is the precise test;
					//  for now the tag "SurveillanceSource" is a Blueprint-assignable proxy.)
					if (Other->ActorHasTag(FName("SurveillanceSource")))
					{
						bWatched = true;
						break;
					}
				}
			}
		}

		bUnderSurveillance = bWatched;
	}
}

void USuspicionComponent::ReportSuspicionEvent(ESuspicionEvent Event)
{
	float Weight = USurveillanceSystem::GetDefaultEventWeight(Event);
	ReportSuspicionEventWithWeight(Event, Weight);
}

void USuspicionComponent::ReportSuspicionEventWithWeight(ESuspicionEvent Event, float CustomWeight)
{
	PersonalSuspicion = FMath::Clamp(PersonalSuspicion + CustomWeight, 0.0f, 1.0f);

	// Forward to global SurveillanceSystem
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USurveillanceSystem* Surveillance = GI->GetSubsystem<USurveillanceSystem>())
		{
			Surveillance->ReportIncident(Event, CustomWeight);
		}
	}
}

void USuspicionComponent::ApplySuspicionDecay(float DeltaTime)
{
	if (PersonalSuspicion > 0.0f && !bUnderSurveillance)
	{
		PersonalSuspicion = FMath::Max(0.0f, PersonalSuspicion - SuspicionDecayRate * DeltaTime);
	}
}
