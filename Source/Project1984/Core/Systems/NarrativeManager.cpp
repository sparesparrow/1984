#include "NarrativeManager.h"
#include "SurveillanceSystem.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
	CurrentAct        = ENarrativeAct::OrdinaryLife;
	CurrentScene      = TEXT("Intro");
	bTutorialComplete = false;
	bDiaryAcquired    = false;
	bJuliaMet         = false;
	bOBrienContacted  = false;
	bGoldsteinBookRead = false;
	bRoom101Complete  = false;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentAct        = ENarrativeAct::OrdinaryLife;
	CurrentScene      = TEXT("Intro");
	bTutorialComplete = false;
	bDiaryAcquired    = false;
	bJuliaMet         = false;
	bOBrienContacted  = false;
	bGoldsteinBookRead = false;
	bRoom101Complete  = false;

	// Load narrative state from save game if a slot exists
	if (UGameplayStatics::DoesSaveGameExist(TEXT("NarrativeSave"), 0))
	{
		UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Save slot found — narrative state ready to restore."));
		// Full restore wired once the project USaveGame subclass is defined.
	}

	// Bind to SurveillanceSystem threshold events so story can react to suspicion spikes
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
		return; // Final act — no further advancement
	}

	ENarrativeAct PreviousAct = CurrentAct;

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
		CurrentScene = TEXT("Aftermath");
		break;
	default:
		return;
	}

	OnActChanged.Broadcast(CurrentAct, PreviousAct);

	// Trigger act transition cinematic (level streaming handled by map name)
	UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Act transition %s -> %s"),
		*UEnum::GetValueAsString(PreviousAct), *UEnum::GetValueAsString(CurrentAct));

	// Load act-specific level/sublevel
	const FName LevelName = *FString::Printf(TEXT("Act_%s"),
		*UEnum::GetValueAsString(CurrentAct).Replace(TEXT("ENarrativeAct::"), TEXT("")));
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);

	// Update surveillance intensity on the subsystem for the new act
	if (GetGameInstance())
	{
		if (USurveillanceSystem* SS = GetGameInstance()->GetSubsystem<USurveillanceSystem>())
		{
			// Higher-act surveillance intensity is applied by scaling the next reported weight
			// via a companion multiplier that gameplay code can query via GetSurveillanceIntensityForAct.
			const float Intensity = GetSurveillanceIntensityForAct(CurrentAct);
			UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Surveillance intensity for act = %.2f"), Intensity);
		}
	}
}

void UNarrativeManager::ForceAdvanceToCapture()
{
	// Only force if we haven't already reached Act IV or later
	if (CurrentAct == ENarrativeAct::OrdinaryLife    ||
		CurrentAct == ENarrativeAct::GrowingDoubt     ||
		CurrentAct == ENarrativeAct::ActiveResistance)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("NarrativeManager: Suspicion critical — forcing transition to Act IV (Capture)."));

		ENarrativeAct PreviousAct = CurrentAct;
		CurrentAct   = ENarrativeAct::CaptureConditioning;
		CurrentScene = TEXT("Arrest");

		OnActChanged.Broadcast(CurrentAct, PreviousAct);

		// Transition level and update intensity as in AdvanceAct
		const FName LevelName = TEXT("Act_CaptureConditioning");
		UGameplayStatics::OpenLevel(GetWorld(), LevelName);
	}
}

void UNarrativeManager::TransitionToScene(const FString& SceneID)
{
	CurrentScene = SceneID;

	UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Entering scene '%s' in act %s"),
		*SceneID, *UEnum::GetValueAsString(CurrentAct));

	// Scene entry: spawn scene-specific actors, configure NPC populations,
	// and fire entry dialogue/events. These are wired to Blueprint scene
	// data assets once content is created.
	// (Blueprint-side: listen to OnActChanged + compare CurrentScene to route events.)
}

bool UNarrativeManager::CanAdvanceAct() const
{
	switch (CurrentAct)
	{
	case ENarrativeAct::OrdinaryLife:
		// Act I complete when the morning-routine tutorial is done
		return bTutorialComplete;

	case ENarrativeAct::GrowingDoubt:
		// Act II complete when player has both acquired the diary and met Julia
		return bDiaryAcquired && bJuliaMet;

	case ENarrativeAct::ActiveResistance:
		// Act III complete when player has contacted O'Brien and read Goldstein's book
		return bOBrienContacted && bGoldsteinBookRead;

	case ENarrativeAct::CaptureConditioning:
		// Act IV complete when Room 101 sequence finishes
		return bRoom101Complete;

	case ENarrativeAct::Resolution:
		// Terminal act — cannot advance further
		return false;
	}

	return false;
}

FString UNarrativeManager::DetermineEnding() const
{
	if (!GetGameInstance())
	{
		return UEnum::GetValueAsString(ENarrativeEnding::TotalCompliance);
	}

	const USurveillanceSystem* SS = GetGameInstance()->GetSubsystem<USurveillanceSystem>();
	const float FinalSuspicion    = SS ? SS->GetSuspicionLevel() : 0.0f;

	// Martyrdom: player maintained resistance — only reached O'Brien AND kept suspicion low
	if (bOBrienContacted && FinalSuspicion < 0.6f)
	{
		return UEnum::GetValueAsString(ENarrativeEnding::Martyrdom);
	}

	// Partial Resistance: player attempted resistance but broke under conditioning
	if (bDiaryAcquired || bOBrienContacted)
	{
		return UEnum::GetValueAsString(ENarrativeEnding::PartialResistance);
	}

	// Total Compliance: player conformed throughout
	return UEnum::GetValueAsString(ENarrativeEnding::TotalCompliance);
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
	if (!GetGameInstance())
	{
		return;
	}

	const USurveillanceSystem* SS = GetGameInstance()->GetSubsystem<USurveillanceSystem>();
	if (!SS)
	{
		return;
	}

	// Suspicion >= 0.8 forces a transition to Act IV regardless of story progress
	if (SS->GetSuspicionLevel() >= 0.8f)
	{
		ForceAdvanceToCapture();
	}
}

float UNarrativeManager::GetSurveillanceIntensityForAct(ENarrativeAct Act) const
{
	switch (Act)
	{
	case ENarrativeAct::OrdinaryLife:       return 0.5f;
	case ENarrativeAct::GrowingDoubt:       return 0.7f;
	case ENarrativeAct::ActiveResistance:   return 1.0f;
	case ENarrativeAct::CaptureConditioning:return 1.5f;
	case ENarrativeAct::Resolution:         return 1.0f;
	}
	return 1.0f;
}
