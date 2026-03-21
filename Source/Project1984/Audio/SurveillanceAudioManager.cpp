#include "SurveillanceAudioManager.h"

USurveillanceAudioManager::USurveillanceAudioManager()
{
	TensionLevel = EAudioTensionLevel::Calm;
}

void USurveillanceAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TensionLevel = EAudioTensionLevel::Calm;

	// TODO: Load ambient audio layers for each tension level
	// TODO: Start default calm ambient loop (room tone, distant telescreen hum)
}

void USurveillanceAudioManager::SetTensionLevel(EAudioTensionLevel NewLevel)
{
	if (NewLevel != TensionLevel)
	{
		EAudioTensionLevel OldLevel = TensionLevel;
		TensionLevel = NewLevel;
		CrossfadeAmbientLayers(OldLevel, NewLevel);
	}
}

void USurveillanceAudioManager::PlayTelescreenAudio(FVector Location, const FString& BroadcastID)
{
	// TODO: Play spatialized audio at the telescreen's world position
	// TODO: Use directional audio emanating from the screen surface
	// TODO: Content: Party announcements, production stats, war updates
}

void USurveillanceAudioManager::StartTwoMinutesHateAudio()
{
	// TODO: Transition to hate sequence audio:
	// 1. Goldstein's droning voice
	// 2. Crowd fury sounds
	// 3. Rhythmic chanting
	// 4. Climax: Big Brother's face, crowd ecstasy
	// TODO: Spatialized crowd audio around the player
}

void USurveillanceAudioManager::StopTwoMinutesHateAudio()
{
	// TODO: Fade out hate audio
	// TODO: Return to normal ambient level
}

void USurveillanceAudioManager::PlayChestnutTreeSong()
{
	// TODO: Play "Under the Spreading Chestnut Tree" — the song from the ending
	// TODO: This plays during the resolution/debrief sequence
	// TODO: Slow, melancholic arrangement
}

void USurveillanceAudioManager::UpdateAtmosphereFromSuspicion(float SuspicionLevel)
{
	// Map suspicion level to audio tension:
	// 0.0-0.3 → Calm
	// 0.3-0.6 → Uneasy
	// 0.6-0.8 → Tense
	// 0.8-1.0 → Danger
	EAudioTensionLevel NewLevel;
	if (SuspicionLevel < 0.3f)
	{
		NewLevel = EAudioTensionLevel::Calm;
	}
	else if (SuspicionLevel < 0.6f)
	{
		NewLevel = EAudioTensionLevel::Uneasy;
	}
	else if (SuspicionLevel < 0.8f)
	{
		NewLevel = EAudioTensionLevel::Tense;
	}
	else
	{
		NewLevel = EAudioTensionLevel::Danger;
	}

	SetTensionLevel(NewLevel);
}

void USurveillanceAudioManager::CrossfadeAmbientLayers(EAudioTensionLevel FromLevel, EAudioTensionLevel ToLevel)
{
	// TODO: Smoothly crossfade between ambient audio layers
	// TODO: Fade duration: 2-3 seconds for gradual transition
	// TODO: Layers:
	// Calm: room tone, distant telescreen hum, clock ticking
	// Uneasy: + dissonant low drone, occasional footstep
	// Tense: + heartbeat, whispers, surveillance static
	// Danger: + alarm-like drone, boots marching, radio chatter
	// Room101: complete audio override — isolation, fear sounds
}
