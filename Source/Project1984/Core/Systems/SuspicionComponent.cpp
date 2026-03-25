#include "SuspicionComponent.h"
#include "SurveillanceSystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

USuspicionComponent::USuspicionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	PersonalSuspicion   = 0.0f;
	SuspicionDecayRate  = 0.01f; // Per second; slow natural decay
	bUnderSurveillance  = false;
}

void USuspicionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Nothing to register here — this component lives on the surveillance *subject*
	// (Winston). Surveillance *sources* (telescreens, NPCs) register themselves.
	// bUnderSurveillance is set externally by those sources when the player enters view.
}

void USuspicionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Mirror bUnderSurveillance from the global suspicion level:
	// once the Party is actively investigating (>=0.3) the player is never truly unobserved.
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
		{
			bUnderSurveillance = (SS->GlobalSuspicion >= 0.3f) || bUnderSurveillance;
		}
	}

	ApplySuspicionDecay(DeltaTime);
}

void USuspicionComponent::ReportSuspicionEvent(ESuspicionEvent Event)
{
	const float Weight = USurveillanceSystem::GetDefaultEventWeight(Event);
	ReportSuspicionEventWithWeight(Event, Weight);
}

void USuspicionComponent::ReportSuspicionEventWithWeight(ESuspicionEvent Event, float CustomWeight)
{
	PersonalSuspicion = FMath::Clamp(PersonalSuspicion + CustomWeight, 0.0f, 1.0f);

	// Forward to the global subsystem — single source of truth for story state.
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
		{
			SS->ReportIncident(Event, CustomWeight);
		}
	}
}

void USuspicionComponent::ApplySuspicionDecay(float DeltaTime)
{
	// Suspicion decays only when the player is not actively under surveillance.
	// At high global suspicion levels the Party does not forget.
	if (PersonalSuspicion > 0.0f && !bUnderSurveillance)
	{
		PersonalSuspicion = FMath::Max(0.0f, PersonalSuspicion - SuspicionDecayRate * DeltaTime);
	}
}
