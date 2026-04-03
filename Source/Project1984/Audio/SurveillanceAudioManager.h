#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "SurveillanceAudioManager.generated.h"

class USurveillanceSystem;

/**
 * EAudioTensionLevel
 *
 * Audio atmosphere levels tied to suspicion state.
 */
UENUM(BlueprintType)
enum class EAudioTensionLevel : uint8
{
	/** Calm — everyday Airstrip One ambience */
	Calm		UMETA(DisplayName = "Calm"),
	/** Uneasy — subtle dissonant undertones */
	Uneasy		UMETA(DisplayName = "Uneasy"),
	/** Tense — heightened surveillance audio, footsteps, whispers */
	Tense		UMETA(DisplayName = "Tense"),
	/** Danger — alarm-like drones, Thought Police approaching */
	Danger		UMETA(DisplayName = "Danger"),
	/** Room101 — extreme psychological audio (Room 101 sequence) */
	Room101		UMETA(DisplayName = "Room 101")
};

/**
 * USurveillanceAudioManager
 *
 * Manages spatial audio for the 1984 simulation. Handles ambisonic
 * room tones, telescreen directional audio, propaganda broadcasts,
 * and dynamically adjusts audio atmosphere based on suspicion level.
 *
 * Audio assets (USoundBase*, USoundCue*) are assigned via Blueprint
 * subclass properties once content is created. The C++ layer manages
 * the state machine, crossfade logic, and layer descriptions.
 */
UCLASS()
class PROJECT1984_API USurveillanceAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USurveillanceAudioManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Current audio tension level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Audio")
	EAudioTensionLevel TensionLevel;

	/** Set the audio tension level (crossfades between ambient layers) */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void SetTensionLevel(EAudioTensionLevel NewLevel);

	/** Play a telescreen broadcast audio at a world location */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void PlayTelescreenAudio(FVector Location, const FString& BroadcastID);

	/** Start the Two Minutes Hate audio sequence */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void StartTwoMinutesHateAudio();

	/** Stop the Two Minutes Hate audio sequence */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void StopTwoMinutesHateAudio();

	/** Play the "Under the Spreading Chestnut Tree" music (Act V) */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void PlayChestnutTreeSong();

	/** Update audio atmosphere based on current suspicion level */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void UpdateAtmosphereFromSuspicion(float SuspicionLevel);

	/** Callback bound to SurveillanceSystem::OnAudioTensionChanged */
	UFUNCTION()
	void OnAudioTensionChanged(float TensionValue);

	/** Manually set Room 101 extreme audio mode */
	UFUNCTION(BlueprintCallable, Category = "1984|Audio")
	void SetRoom101Mode(bool bEnable);

protected:
	/** Get a human-readable description of each tension layer (for logging and subtitles) */
	static FString GetLayerDescription(EAudioTensionLevel Level);

	/** Crossfade between ambient audio layers */
	void CrossfadeAmbientLayers(EAudioTensionLevel FromLevel, EAudioTensionLevel ToLevel);

	/** Tick crossfade volume interpolation (called by timer) */
	void TickCrossfade();

private:
	// One audio component per tension layer — only one plays at a time,
	// but we keep them all alive to allow instant crossfade back.
	UPROPERTY()
	TMap<uint8, UAudioComponent*> AmbientComponents;

	UPROPERTY()
	UAudioComponent* TelescreenAudioComponent;

	UPROPERTY()
	UAudioComponent* HateAudioComponent;

	UPROPERTY()
	UAudioComponent* ChestnutAudioComponent;

	// Crossfade state.
	FTimerHandle        CrossfadeTimerHandle;
	float               CrossfadeProgress;  // 0.0 → 1.0 over CrossfadeDuration
	EAudioTensionLevel  CrossfadeFrom;
	EAudioTensionLevel  CrossfadeTo;
	static constexpr float CrossfadeDuration = 2.5f;  // seconds
	static constexpr float CrossfadeTickRate = 0.05f; // 20 Hz
};
