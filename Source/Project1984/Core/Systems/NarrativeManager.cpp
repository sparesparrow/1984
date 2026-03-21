#include "NarrativeManager.h"

UNarrativeManager::UNarrativeManager()
{
	CurrentAct = ENarrativeAct::OrdinaryLife;
	CurrentScene = TEXT("Intro");
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentAct = ENarrativeAct::OrdinaryLife;
	CurrentScene = TEXT("Intro");

	// TODO: Load narrative state from save game if continuing
	// TODO: Bind to SurveillanceSystem threshold events
}

void UNarrativeManager::AdvanceAct()
{
	ENarrativeAct PreviousAct = CurrentAct;

	switch (CurrentAct)
	{
	case ENarrativeAct::OrdinaryLife:
		CurrentAct = ENarrativeAct::GrowingDoubt;
		CurrentScene = TEXT("DiaryDiscovery");
		break;
	case ENarrativeAct::GrowingDoubt:
		CurrentAct = ENarrativeAct::ActiveResistance;
		CurrentScene = TEXT("OBrienContact");
		break;
	case ENarrativeAct::ActiveResistance:
		CurrentAct = ENarrativeAct::CaptureConditioning;
		CurrentScene = TEXT("Arrest");
		break;
	case ENarrativeAct::CaptureConditioning:
		CurrentAct = ENarrativeAct::Resolution;
		CurrentScene = TEXT("Aftermath");
		break;
	case ENarrativeAct::Resolution:
		// Final act — no further advancement
		return;
	}

	OnActChanged.Broadcast(CurrentAct, PreviousAct);

	// TODO: Trigger act transition cinematic
	// TODO: Load act-specific level/sublevel
	// TODO: Update surveillance intensity
}

void UNarrativeManager::TransitionToScene(const FString& SceneID)
{
	CurrentScene = SceneID;

	// TODO: Load scene-specific actors and triggers
	// TODO: Update NPC populations for the scene
	// TODO: Trigger scene entry dialogue/events
}

bool UNarrativeManager::CanAdvanceAct() const
{
	// TODO: Check act-specific completion conditions:
	// Act I: Complete morning routine tutorial
	// Act II: Acquire diary, meet Julia
	// Act III: Meet O'Brien, read Goldstein's book
	// Act IV: Complete Room 101 sequence
	return false;
}

FString UNarrativeManager::DetermineEnding() const
{
	// TODO: Evaluate player's cumulative suspicion history and decisions
	// Possible endings:
	// "TotalCompliance" — Player fully conforms, loves Big Brother
	// "PartialResistance" — Player resisted but ultimately broke
	// "Martyrdom" — Player maintained resistance (rare, highest difficulty)

	return TEXT("TotalCompliance");
}

void UNarrativeManager::EvaluateNarrativeTriggers()
{
	// TODO: Check global suspicion for forced narrative advances
	// e.g., suspicion > 0.8 forces transition to Act IV (capture)
}
