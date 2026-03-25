#include "SurveillanceAudioManager.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Sound/SoundBase.h"

USurveillanceAudioManager::USurveillanceAudioManager()
{
	TensionLevel      = EAudioTensionLevel::Calm;
	CrossfadeProgress = 0.0f;
	CrossfadeFrom     = EAudioTensionLevel::Calm;
	CrossfadeTo       = EAudioTensionLevel::Calm;

	TelescreenAudioComponent = nullptr;
	HateAudioComponent       = nullptr;
	ChestnutAudioComponent   = nullptr;
}

void USurveillanceAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TensionLevel = EAudioTensionLevel::Calm;

	// Bind to SurveillanceSystem::OnAudioTensionChanged so audio reacts
	// automatically whenever suspicion crosses a threshold.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
		{
			SS->OnAudioTensionChanged.AddDynamic(
				this, &USurveillanceAudioManager::OnAudioTensionChanged);
		}
	}

	// Prime the ambient component map with null entries so code paths can key safely.
	for (uint8 Level = 0; Level <= static_cast<uint8>(EAudioTensionLevel::Room101); ++Level)
	{
		AmbientComponents.Add(Level, nullptr);
	}

	UE_LOG(LogTemp, Log, TEXT("SurveillanceAudioManager: Initialized. Layer: %s"),
		*GetLayerDescription(TensionLevel));
}

void USurveillanceAudioManager::Deinitialize()
{
	// Unbind delegate before destruction to avoid dangling references.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
		{
			SS->OnAudioTensionChanged.RemoveDynamic(
				this, &USurveillanceAudioManager::OnAudioTensionChanged);
		}
	}

	Super::Deinitialize();
}

void USurveillanceAudioManager::SetTensionLevel(EAudioTensionLevel NewLevel)
{
	if (NewLevel == TensionLevel) return;

	const EAudioTensionLevel OldLevel = TensionLevel;
	TensionLevel = NewLevel;
	CrossfadeAmbientLayers(OldLevel, NewLevel);
}

void USurveillanceAudioManager::OnAudioTensionChanged(float TensionValue)
{
	UpdateAtmosphereFromSuspicion(TensionValue);
}

void USurveillanceAudioManager::PlayTelescreenAudio(FVector Location, const FString& BroadcastID)
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;

	// Stop any previous telescreen broadcast.
	if (TelescreenAudioComponent && TelescreenAudioComponent->IsPlaying())
	{
		TelescreenAudioComponent->FadeOut(0.3f, 0.0f);
	}

	// Spatialized audio at the telescreen's world position.
	// Blueprint subclass plays the relevant USoundAttenuation asset here.
	UE_LOG(LogTemp, Verbose,
		TEXT("SurveillanceAudioManager: Telescreen broadcast '%s' at (%.0f, %.0f, %.0f)."),
		*BroadcastID, Location.X, Location.Y, Location.Z);
}

void USurveillanceAudioManager::StartTwoMinutesHateAudio()
{
	// Two Minutes Hate audio sequence — force tension to Tense for the sequence.
	SetTensionLevel(EAudioTensionLevel::Tense);

	UE_LOG(LogTemp, Log,
		TEXT("SurveillanceAudioManager: Two Minutes Hate audio STARTED."));
}

void USurveillanceAudioManager::StopTwoMinutesHateAudio()
{
	if (HateAudioComponent && HateAudioComponent->IsPlaying())
	{
		HateAudioComponent->FadeOut(/*FadeOutDuration=*/2.0f, /*FadeVolumeLevel=*/0.0f);
	}

	// Return to the suspicion-appropriate ambient layer.
	SetTensionLevel(EAudioTensionLevel::Calm);

	UE_LOG(LogTemp, Log,
		TEXT("SurveillanceAudioManager: Two Minutes Hate audio STOPPED. Returning to ambient."));
}

void USurveillanceAudioManager::PlayChestnutTreeSong()
{
	// "Under the Spreading Chestnut Tree" — the melancholic song in Act V.
	UE_LOG(LogTemp, Log,
		TEXT("SurveillanceAudioManager: Playing 'Under the Spreading Chestnut Tree' (Act V)."));

	SetTensionLevel(EAudioTensionLevel::Calm);
}

void USurveillanceAudioManager::SetRoom101Mode(bool bEnable)
{
	SetTensionLevel(bEnable ? EAudioTensionLevel::Room101 : EAudioTensionLevel::Tense);
}

void USurveillanceAudioManager::UpdateAtmosphereFromSuspicion(float SuspicionLevel)
{
	EAudioTensionLevel NewLevel;
	if      (SuspicionLevel < 0.3f) NewLevel = EAudioTensionLevel::Calm;
	else if (SuspicionLevel < 0.6f) NewLevel = EAudioTensionLevel::Uneasy;
	else if (SuspicionLevel < 0.8f) NewLevel = EAudioTensionLevel::Tense;
	else                            NewLevel = EAudioTensionLevel::Danger;

	SetTensionLevel(NewLevel);
}

void USurveillanceAudioManager::CrossfadeAmbientLayers(EAudioTensionLevel FromLevel, EAudioTensionLevel ToLevel)
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;

	// Cancel any in-progress crossfade.
	World->GetTimerManager().ClearTimer(CrossfadeTimerHandle);

	CrossfadeFrom     = FromLevel;
	CrossfadeTo       = ToLevel;
	CrossfadeProgress = 0.0f;

	// Activate the incoming layer at zero volume so we can fade it up.
	UAudioComponent* IncomingComp = AmbientComponents.FindRef(static_cast<uint8>(ToLevel));
	if (IncomingComp)
	{
		IncomingComp->SetVolumeMultiplier(0.0f);
		if (!IncomingComp->IsPlaying()) IncomingComp->Play();
	}

	// Start the timer-based volume interpolation (20 Hz).
	World->GetTimerManager().SetTimer(
		CrossfadeTimerHandle,
		this,
		&USurveillanceAudioManager::TickCrossfade,
		CrossfadeTickRate,
		/*bLoop=*/true);

	UE_LOG(LogTemp, Log,
		TEXT("SurveillanceAudioManager: Crossfading audio %s -> %s (%.1fs)."),
		*GetLayerDescription(FromLevel), *GetLayerDescription(ToLevel), CrossfadeDuration);
}

void USurveillanceAudioManager::TickCrossfade()
{
	CrossfadeProgress += CrossfadeTickRate / CrossfadeDuration;
	CrossfadeProgress  = FMath::Clamp(CrossfadeProgress, 0.0f, 1.0f);

	const float AlphaIn  = CrossfadeProgress;
	const float AlphaOut = 1.0f - CrossfadeProgress;

	UAudioComponent* OutComp = AmbientComponents.FindRef(static_cast<uint8>(CrossfadeFrom));
	UAudioComponent* InComp  = AmbientComponents.FindRef(static_cast<uint8>(CrossfadeTo));

	if (OutComp && OutComp->IsPlaying()) OutComp->SetVolumeMultiplier(AlphaOut);
	if (InComp  && InComp->IsPlaying())  InComp->SetVolumeMultiplier(AlphaIn);

	if (CrossfadeProgress >= 1.0f)
	{
		if (OutComp && OutComp->IsPlaying()) OutComp->Stop();

		UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
		if (World) World->GetTimerManager().ClearTimer(CrossfadeTimerHandle);
	}
}

FString USurveillanceAudioManager::GetLayerDescription(EAudioTensionLevel Level)
{
	switch (Level)
	{
	case EAudioTensionLevel::Calm:    return TEXT("Calm (room tone, telescreen hum)");
	case EAudioTensionLevel::Uneasy:  return TEXT("Uneasy (dissonant drone, creaking)");
	case EAudioTensionLevel::Tense:   return TEXT("Tense (heartbeat, whispers, static)");
	case EAudioTensionLevel::Danger:  return TEXT("Danger (alarm drone, marching boots)");
	case EAudioTensionLevel::Room101: return TEXT("Room 101 (isolation, fear stimuli)");
	}
	return TEXT("Unknown");
}
