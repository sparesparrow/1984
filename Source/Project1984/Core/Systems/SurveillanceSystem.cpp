#include "SurveillanceSystem.h"
#include "NarrativeManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

USurveillanceSystem::USurveillanceSystem()
{
	GlobalSuspicion = 0.0f;
	PreviousSuspicionLevel = 0.0f;
}

void USurveillanceSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GlobalSuspicion = 0.0f;
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

	// Check for threshold crossings
	const float Thresholds[] = { 0.3f, 0.6f, 0.8f };
	for (float Threshold : Thresholds)
	{
		bool bCrossedUp   = PreviousSuspicionLevel < Threshold && GlobalSuspicion >= Threshold;
		bool bCrossedDown = PreviousSuspicionLevel >= Threshold && GlobalSuspicion < Threshold;
		if (bCrossedUp || bCrossedDown)
		{
			OnSuspicionThresholdChanged.Broadcast(GlobalSuspicion, PreviousSuspicionLevel);
			break;
		}
	}

	// Log incident to decision history for educational export
	const float WorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const FString EventName = UEnum::GetValueAsString(Event);
	DecisionHistory.Add(FString::Printf(
		TEXT("[%.2fs] %s  weight=%.2f  suspicion=%.2f"),
		WorldTime, *EventName, Weight, GlobalSuspicion));

	UpdateStoryState();
}

void USurveillanceSystem::UpdateStoryState()
{
	if (!GetGameInstance())
	{
		return;
	}

	// Notify NarrativeManager of suspicion-driven state changes.
	// 0.0–0.3: Unsuspected — standard behavior
	// 0.3–0.6: Under observation — increased NPC attention
	// 0.6–0.8: Active investigation — Thought Police dispatched
	// 0.8–1.0: Imminent capture — force Act IV transition
	if (UNarrativeManager* NarrativeManager =
			GetGameInstance()->GetSubsystem<UNarrativeManager>())
	{
		if (GlobalSuspicion >= 0.8f)
		{
			NarrativeManager->ForceAdvanceToCapture();
		}
	}

	// Broadcast updated tension level for audio and visual systems
	// (SurveillanceAudioManager and TelescreenComponent listen to this)
	OnAudioTensionChanged.Broadcast(GlobalSuspicion);
}

float USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent Event)
{
	switch (Event)
	{
	case ESuspicionEvent::WritingDiary:				return  0.15f;
	case ESuspicionEvent::FacialExpression:			return  0.05f;
	case ESuspicionEvent::EnteringOffLimits:		return  0.25f;
	case ESuspicionEvent::AttendingTwoMinutesHate:	return -0.10f;
	case ESuspicionEvent::SpeakingNewspeak:			return -0.08f;
	case ESuspicionEvent::SpeakingOldspeak:			return  0.12f;
	case ESuspicionEvent::CitizenReport:			return  0.15f;
	case ESuspicionEvent::ThoughtPoliceDetection:	return  0.35f;
	case ESuspicionEvent::TelescreenObservation:	return  0.08f;
	case ESuspicionEvent::SecretMeeting:			return  0.20f;
	default:										return  0.0f;
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
