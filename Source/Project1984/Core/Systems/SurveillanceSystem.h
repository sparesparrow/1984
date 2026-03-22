#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SurveillanceSystem.generated.h"

/**
 * ESuspicionEvent
 *
 * Events that raise or lower the global suspicion meter.
 * Weights defined per Section 4.1 of the VR/XR Development Plan.
 */
UENUM(BlueprintType)
enum class ESuspicionEvent : uint8
{
	/** Writing in the forbidden diary (+0.15) */
	WritingDiary			UMETA(DisplayName = "Writing in Diary"),
	/** Facial micro-expression detected via eye tracking (+0.05) */
	FacialExpression		UMETA(DisplayName = "Facial Micro-Expression"),
	/** Entering a restricted/off-limits area (+0.25) */
	EnteringOffLimits		UMETA(DisplayName = "Entering Off-Limits Area"),
	/** Attending Two Minutes Hate (-0.10, reduces suspicion) */
	AttendingTwoMinutesHate	UMETA(DisplayName = "Attending Two Minutes Hate"),
	/** Speaking Newspeak correctly (-0.08, reduces suspicion) */
	SpeakingNewspeak		UMETA(DisplayName = "Speaking Newspeak Correctly"),
	/** Speaking Oldspeak in public (+0.12) */
	SpeakingOldspeak		UMETA(DisplayName = "Speaking Oldspeak"),
	/** Reported by citizen NPC (+0.10 to +0.30) */
	CitizenReport			UMETA(DisplayName = "Citizen Report"),
	/** Caught by Thought Police vision cone (+0.35) */
	ThoughtPoliceDetection	UMETA(DisplayName = "Thought Police Detection"),
	/** Observed by telescreen (+0.08) */
	TelescreenObservation	UMETA(DisplayName = "Telescreen Observation"),
	/** Meeting with Julia secretly (+0.20) */
	SecretMeeting			UMETA(DisplayName = "Secret Meeting")
};

/**
 * ISurveillanceSource
 *
 * Interface for actors that contribute to the surveillance network
 * (telescreens, NPCs, Thought Police patrols).
 */
UINTERFACE(MinimalAPI, BlueprintType)
class USurveillanceSource : public UInterface
{
	GENERATED_BODY()
};

class ISurveillanceSource
{
	GENERATED_BODY()

public:
	/** Whether this source is currently active and monitoring */
	virtual bool IsActivelyMonitoring() const = 0;

	/** Get the surveillance coverage area */
	virtual float GetCoverageRadius() const = 0;
};

/**
 * USurveillanceSystem
 *
 * The heart of the 1984 simulation. Manages all surveillance entities,
 * tracks the global suspicion level (0.0-1.0), and triggers narrative
 * branches at defined thresholds.
 *
 * Suspicion thresholds:
 *   0.0 - 0.3: Unsuspected — normal daily life
 *   0.3 - 0.6: Under observation — increased NPC attention
 *   0.6 - 0.8: Active investigation — Thought Police dispatched
 *   0.8 - 1.0: Imminent capture — leads to Room 101
 */
UCLASS()
class PROJECT1984_API USurveillanceSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USurveillanceSystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Global suspicion level: 0.0 (safe) to 1.0 (imminent capture) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Surveillance")
	float GlobalSuspicion;

	/** Register a surveillance actor (telescreen, NPC, etc.) */
	UFUNCTION(BlueprintCallable, Category = "1984|Surveillance")
	void RegisterSurveillanceActor(AActor* Source);

	/** Unregister a surveillance actor */
	UFUNCTION(BlueprintCallable, Category = "1984|Surveillance")
	void UnregisterSurveillanceActor(AActor* Source);

	/** Report a suspicion event with its predefined weight */
	UFUNCTION(BlueprintCallable, Category = "1984|Surveillance")
	void ReportIncident(ESuspicionEvent Event, float Weight);

	/** Update story state based on current suspicion thresholds */
	UFUNCTION(BlueprintCallable, Category = "1984|Surveillance")
	void UpdateStoryState();

	/** Get the default weight for a suspicion event type */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Surveillance")
	static float GetDefaultEventWeight(ESuspicionEvent Event);

	/** Get number of active surveillance sources */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Surveillance")
	int32 GetActiveSurveillanceCount() const;

	/** Delegate broadcast when suspicion threshold changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSuspicionThresholdChanged, float, NewLevel, float, OldLevel);

	UPROPERTY(BlueprintAssignable, Category = "1984|Surveillance")
	FOnSuspicionThresholdChanged OnSuspicionThresholdChanged;

protected:
	/** All registered surveillance sources in the current level */
	UPROPERTY()
	TArray<AActor*> SurveillanceActors;

	/** Previous suspicion level for threshold change detection */
	float PreviousSuspicionLevel;
};
