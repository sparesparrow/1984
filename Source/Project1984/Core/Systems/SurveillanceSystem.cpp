#include "SurveillanceSystem.h"
#include "NarrativeManager.h"
#include "Project1984/Core/GameMode/Project1984GameState.h"
#include "Project1984/Audio/SurveillanceAudioManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

USurveillanceSystem::USurveillanceSystem()
{
	GlobalSuspicion        = 0.0f;
	PreviousSuspicionLevel = 0.0f;
}

void USurveillanceSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Ensure dependents are initialized before us so we can call them immediately.
	Collection.InitializeDependency<UNarrativeManager>();
	Collection.InitializeDependency<USurveillanceAudioManager>();

	Super::Initialize(Collection);

	GlobalSuspicion        = 0.0f;
	PreviousSuspicionLevel = 0.0f;
	SurveillanceActors.Empty();
	DecisionHistory.Empty();

	// Load suspicion state from save game if a slot exists
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		if (USaveGame* SaveData = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0))
		{
			// Full restoration will be wired to the project-specific save class.
			// For now flag the successful load so systems can react.
			UE_LOG(LogTemp, Log, TEXT("SurveillanceSystem: Save slot found — suspicion state ready to restore."));
		}
	}
}

void USurveillanceSystem::Deinitialize()
{
	SurveillanceActors.Empty();
	Super::Deinitialize();
}

void USurveillanceSystem::RegisterSurveillanceActor(AActor* Source)
{
	if (Source && !SurveillanceActors.Contains(Source))
	{
		SurveillanceActors.Add(Source);
	}
}

void USurveillanceSystem::UnregisterSurveillanceActor(AActor* Source)
{
	SurveillanceActors.Remove(Source);
}

void USurveillanceSystem::ReportIncident(ESuspicionEvent Event, float Weight)
{
	PreviousSuspicionLevel = GlobalSuspicion;
	GlobalSuspicion = FMath::Clamp(GlobalSuspicion + Weight, 0.0f, 1.0f);

	// Detect threshold crossings and broadcast the delegate.
	static constexpr float Thresholds[] = { 0.3f, 0.6f, 0.8f };
	for (float T : Thresholds)
	{
		if ((PreviousSuspicionLevel < T && GlobalSuspicion >= T) ||
		    (PreviousSuspicionLevel >= T && GlobalSuspicion < T))
		{
			OnSuspicionThresholdChanged.Broadcast(GlobalSuspicion, PreviousSuspicionLevel);
			break;
		}
	}

	// Log to decision history (educational export) and count thoughtcrimes.
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (AProject1984GameState* GS = World->GetGameState<AProject1984GameState>())
		{
			GS->GlobalSuspicionLevel = GlobalSuspicion;

			const UEnum* EventEnum = StaticEnum<ESuspicionEvent>();
			const FString EventName = EventEnum
				? EventEnum->GetNameStringByValue(static_cast<int64>(Event))
				: TEXT("Unknown");

			GS->RecordDecision(
				FString::Printf(TEXT("Incident_%s"), *EventName),
				FString::Printf(TEXT("GlobalSuspicion=%.3f"), GlobalSuspicion));
		}
	}

	UpdateStoryState();
}

void USurveillanceSystem::UpdateStoryState()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	// --- Narrative branch ---
	// 0.8+ = capture threshold: force transition to Act IV if not already there.
	if (GlobalSuspicion >= 0.8f)
	{
		if (UNarrativeManager* NM = GI->GetSubsystem<UNarrativeManager>())
		{
			const bool bNotYetCaptured =
				NM->CurrentAct != ENarrativeAct::CaptureConditioning &&
				NM->CurrentAct != ENarrativeAct::Resolution;

			if (bNotYetCaptured)
			{
				// Advance acts until we reach CaptureConditioning.
				while (NM->CurrentAct < ENarrativeAct::CaptureConditioning)
				{
					NM->AdvanceAct();
				}
			}
		}
	}

	// --- Audio atmosphere ---
	// Maps suspicion bands → tension levels (see SurveillanceAudioManager).
	if (USurveillanceAudioManager* Audio = GI->GetSubsystem<USurveillanceAudioManager>())
	{
		Audio->UpdateAtmosphereFromSuspicion(GlobalSuspicion);
	}
}

float USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent Event)
{
	switch (Event)
	{
	case ESuspicionEvent::WritingDiary:              return  0.15f;
	case ESuspicionEvent::FacialExpression:          return  0.05f;
	case ESuspicionEvent::EnteringOffLimits:         return  0.25f;
	case ESuspicionEvent::AttendingTwoMinutesHate:   return -0.10f;
	case ESuspicionEvent::SpeakingNewspeak:          return -0.08f;
	case ESuspicionEvent::SpeakingOldspeak:          return  0.12f;
	case ESuspicionEvent::CitizenReport:             return  0.15f;
	case ESuspicionEvent::ThoughtPoliceDetection:    return  0.35f;
	case ESuspicionEvent::TelescreenObservation:     return  0.08f;
	case ESuspicionEvent::SecretMeeting:             return  0.20f;
	default:                                         return  0.0f;
	}
}

int32 USurveillanceSystem::GetActiveSurveillanceCount() const
{
	return SurveillanceActors.Num();
}

void USurveillanceSystem::SaveSuspicionState()
{
	// Placeholder: full save will use the project-specific USaveGame subclass.
	// Calling CreateSaveGameObject / AsyncSaveGameToSlot goes here once the
	// save class is defined.
	UE_LOG(LogTemp, Log, TEXT("SurveillanceSystem: SaveSuspicionState called (save class pending)."));
}
