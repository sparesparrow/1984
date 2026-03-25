#include "NarrativeManager.h"
#include "SurveillanceSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Project1984/Core/GameMode/Project1984GameState.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UNarrativeManager::UNarrativeManager()
{
	CurrentAct         = ENarrativeAct::OrdinaryLife;
	CurrentScene       = TEXT("Intro");
	bTutorialComplete  = false;
	bDiaryAcquired     = false;
	bJuliaMet          = false;
	bOBrienContacted   = false;
	bGoldsteinBookRead = false;
	bRoom101Complete   = false;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentAct         = ENarrativeAct::OrdinaryLife;
	CurrentScene       = TEXT("Intro");
	bTutorialComplete  = false;
	bDiaryAcquired     = false;
	bJuliaMet          = false;
	bOBrienContacted   = false;
	bGoldsteinBookRead = false;
	bRoom101Complete   = false;

	// Load narrative state from save game if a slot exists.
	if (UGameplayStatics::DoesSaveGameExist(TEXT("NarrativeSave"), 0))
	{
		UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Save slot found — narrative state ready to restore."));
	}

	// Bind to SurveillanceSystem threshold events so story can react to suspicion spikes.
	if (GetGameInstance())
	{
		if (USurveillanceSystem* SS = GetGameInstance()->GetSubsystem<USurveillanceSystem>())
		{
			SS->OnSuspicionThresholdChanged.AddDynamic(
				this, &UNarrativeManager::OnSuspicionLevelChanged);
		}
	}
}

void UNarrativeManager::AdvanceAct()
{
	if (CurrentAct == ENarrativeAct::Resolution)
	{
		return; // Terminal act — no further advancement.
	}

	const ENarrativeAct PreviousAct = CurrentAct;

	switch (CurrentAct)
	{
	case ENarrativeAct::OrdinaryLife:
		CurrentAct   = ENarrativeAct::GrowingDoubt;
		CurrentScene = TEXT("DiaryDiscovery");
		break;
	case ENarrativeAct::GrowingDoubt:
		CurrentAct   = ENarrativeAct::ActiveResistance;
		CurrentScene = TEXT("OBrienContact");
		break;
	case ENarrativeAct::ActiveResistance:
		CurrentAct   = ENarrativeAct::CaptureConditioning;
		CurrentScene = TEXT("Arrest");
		break;
	case ENarrativeAct::CaptureConditioning:
		CurrentAct   = ENarrativeAct::Resolution;
		CurrentScene = FString::Printf(TEXT("Ending_%s"), *DetermineEnding());
		break;
	default:
		return;
	}

	OnActChanged.Broadcast(CurrentAct, PreviousAct);
	EvaluateNarrativeTriggers();

	UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Act transition %d -> %d  Scene='%s'"),
		static_cast<int32>(PreviousAct), static_cast<int32>(CurrentAct), *CurrentScene);
}

void UNarrativeManager::ForceAdvanceToCapture()
{
	if (CurrentAct == ENarrativeAct::OrdinaryLife    ||
		CurrentAct == ENarrativeAct::GrowingDoubt     ||
		CurrentAct == ENarrativeAct::ActiveResistance)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("NarrativeManager: Suspicion critical — forcing transition to Act IV (Capture)."));

		const ENarrativeAct PreviousAct = CurrentAct;
		CurrentAct   = ENarrativeAct::CaptureConditioning;
		CurrentScene = TEXT("Arrest");

		OnActChanged.Broadcast(CurrentAct, PreviousAct);
	}
}

void UNarrativeManager::TransitionToScene(const FString& SceneID)
{
	CurrentScene = SceneID;

	UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Entering scene '%s' in act %d"),
		*SceneID, static_cast<int32>(CurrentAct));
}

bool UNarrativeManager::CanAdvanceAct() const
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return false;

	const AProject1984GameState* GS = World->GetGameState<AProject1984GameState>();
	if (!GS) return false;

	switch (CurrentAct)
	{
	case ENarrativeAct::OrdinaryLife:
		// Act I ends when tutorial is complete. Flag set by BP trigger volumes.
		return bTutorialComplete;

	case ENarrativeAct::GrowingDoubt:
		// Act II → III requires Julia contact.
		return bJuliaMet || GS->bJuliaContactMade;

	case ENarrativeAct::ActiveResistance:
		// Act III → IV if O'Brien meeting complete OR Party forces capture.
		return (bOBrienContacted && bGoldsteinBookRead) ||
		       GS->bOBrienMeetingComplete ||
		       GS->GlobalSuspicionLevel >= 0.8f;

	case ENarrativeAct::CaptureConditioning:
		// Act IV → V when Room 101 sequence finishes.
		return bRoom101Complete;

	case ENarrativeAct::Resolution:
		return false;
	}

	return false;
}

FString UNarrativeManager::DetermineEnding() const
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return TEXT("TotalCompliance");

	const AProject1984GameState* GS = World->GetGameState<AProject1984GameState>();
	if (!GS) return TEXT("TotalCompliance");

	// "TotalCompliance" — player never committed a recorded thoughtcrime.
	if (GS->ThoughtcrimeCount == 0)
	{
		return TEXT("TotalCompliance");
	}

	// "Martyrdom" — player maintained resistance, reached O'Brien, kept suspicion moderate.
	if ((bOBrienContacted || GS->bOBrienMeetingComplete) && GS->GlobalSuspicionLevel < 0.6f)
	{
		return TEXT("Martyrdom");
	}

	// "PartialResistance" — player resisted but broke under conditioning.
	if (bDiaryAcquired || bOBrienContacted || GS->bJuliaContactMade)
	{
		return TEXT("PartialResistance");
	}

	return TEXT("TotalCompliance");
}

void UNarrativeManager::OnSuspicionLevelChanged(float NewLevel, float OldLevel)
{
	UE_LOG(LogTemp, Log,
		TEXT("NarrativeManager: Suspicion threshold crossed: %.2f -> %.2f"),
		OldLevel, NewLevel);

	EvaluateNarrativeTriggers();
}

void UNarrativeManager::EvaluateNarrativeTriggers()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;

	const AProject1984GameState* GS = World->GetGameState<AProject1984GameState>();
	if (!GS) return;

	// Forced capture: high suspicion overrides normal story gating.
	if (GS->GlobalSuspicionLevel >= 0.8f &&
	    CurrentAct != ENarrativeAct::CaptureConditioning &&
	    CurrentAct != ENarrativeAct::Resolution)
	{
		while (CurrentAct < ENarrativeAct::CaptureConditioning)
		{
			AdvanceAct();
		}
	}
}

float UNarrativeManager::GetSurveillanceIntensityForAct(ENarrativeAct Act) const
{
	switch (Act)
	{
	case ENarrativeAct::OrdinaryLife:        return 0.5f;
	case ENarrativeAct::GrowingDoubt:        return 0.7f;
	case ENarrativeAct::ActiveResistance:    return 1.0f;
	case ENarrativeAct::CaptureConditioning: return 1.5f;
	case ENarrativeAct::Resolution:          return 1.0f;
	}
	return 1.0f;
}
