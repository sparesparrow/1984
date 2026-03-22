#include "SuspicionComponent.h"

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

	// TODO: Register with SurveillanceSystem on begin play
}

void USuspicionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplySuspicionDecay(DeltaTime);

	// TODO: Check if owner is within surveillance coverage area
	// TODO: Update bUnderSurveillance based on line-of-sight from telescreens/NPCs
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
