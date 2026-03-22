#include "SurveillanceSystem.h"

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

	// TODO: Load suspicion state from save game if continuing
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
		bool bCrossedUp = PreviousSuspicionLevel < Threshold && GlobalSuspicion >= Threshold;
		bool bCrossedDown = PreviousSuspicionLevel >= Threshold && GlobalSuspicion < Threshold;
		if (bCrossedUp || bCrossedDown)
		{
			OnSuspicionThresholdChanged.Broadcast(GlobalSuspicion, PreviousSuspicionLevel);
			break;
		}
	}

	UpdateStoryState();

	// TODO: Log incident to decision history for educational export
	// TODO: Trigger visual/audio feedback (telescreen flicker, ambient tension change)
}

void USurveillanceSystem::UpdateStoryState()
{
	// TODO: Branch narrative based on suspicion thresholds:
	// 0.0-0.3: Normal life — standard NPC behavior
	// 0.3-0.6: Under observation — NPCs glance at player, increased patrols
	// 0.6-0.8: Active investigation — Thought Police dispatched, restricted movement
	// 0.8-1.0: Imminent capture — triggers Act IV transition to Room 101

	// TODO: Notify NarrativeManager of state changes
	// TODO: Adjust ambient audio tension level
	// TODO: Update telescreen behavior (more frequent direct addresses)
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
