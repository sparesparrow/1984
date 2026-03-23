#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NarrativeManager.generated.h"

/**
 * ENarrativeAct
 *
 * The five-act structure from the VR/XR Development Plan.
 */
UENUM(BlueprintType)
enum class ENarrativeAct : uint8
{
	/** Act I: Ordinary life — introduction, tutorial */
	OrdinaryLife		UMETA(DisplayName = "Act I - Ordinary Life"),
	/** Act II: Growing doubt — diary, Julia, Brotherhood rumours */
	GrowingDoubt		UMETA(DisplayName = "Act II - Growing Doubt"),
	/** Act III: Active resistance — meeting O'Brien, the book */
	ActiveResistance	UMETA(DisplayName = "Act III - Active Resistance"),
	/** Act IV: Capture and conditioning — Room 101 */
	CaptureConditioning	UMETA(DisplayName = "Act IV - Capture & Conditioning"),
	/** Act V: Resolution — multiple endings based on suspicion history */
	Resolution			UMETA(DisplayName = "Act V - Resolution")
};

/**
 * ENarrativeEnding
 *
 * Possible endings determined by cumulative player decisions.
 */
UENUM(BlueprintType)
enum class ENarrativeEnding : uint8
{
	/** Player fully conformed — loves Big Brother */
	TotalCompliance		UMETA(DisplayName = "Total Compliance"),
	/** Player resisted but ultimately broke under conditioning */
	PartialResistance	UMETA(DisplayName = "Partial Resistance"),
	/** Player maintained resistance throughout (rare, hardest path) */
	Martyrdom			UMETA(DisplayName = "Martyrdom")
};

/**
 * UNarrativeManager
 *
 * Scene-graph-based story system with consequence tracking.
 * Manages the five-act narrative structure, scene transitions,
 * and determines which ending the player reaches based on their
 * cumulative suspicion history and decisions.
 */
UCLASS()
class PROJECT1984_API UNarrativeManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UNarrativeManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---------------------------------------------------------------
	// State
	// ---------------------------------------------------------------

	/** Current narrative act */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	ENarrativeAct CurrentAct;

	/** Current scene within the act */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	FString CurrentScene;

	// ---------------------------------------------------------------
	// Act completion flags (set by gameplay systems)
	// ---------------------------------------------------------------

	/** Act I complete: morning routine tutorial finished */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1984|Narrative|Progress")
	bool bTutorialComplete;

	/** Act II flag: player has acquired the diary */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1984|Narrative|Progress")
	bool bDiaryAcquired;

	/** Act II flag: player has met Julia */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1984|Narrative|Progress")
	bool bJuliaMet;

	/** Act III flag: player has contacted O'Brien */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1984|Narrative|Progress")
	bool bOBrienContacted;

	/** Act III flag: player has read Goldstein's book */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1984|Narrative|Progress")
	bool bGoldsteinBookRead;

	/** Act IV flag: Room 101 sequence completed */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "1984|Narrative|Progress")
	bool bRoom101Complete;

	// ---------------------------------------------------------------
	// Methods
	// ---------------------------------------------------------------

	/** Advance to the next narrative act */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	void AdvanceAct();

	/** Force transition to Act IV (capture) — called by SurveillanceSystem at suspicion >= 0.8 */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	void ForceAdvanceToCapture();

	/** Transition to a specific scene within the current act */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	void TransitionToScene(const FString& SceneID);

	/** Check if narrative conditions are met to advance to the next act */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Narrative")
	bool CanAdvanceAct() const;

	/** Get the ending type based on player history */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Narrative")
	FString DetermineEnding() const;

	/** Delegate for act transitions */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActChanged, ENarrativeAct, NewAct, ENarrativeAct, PreviousAct);

	UPROPERTY(BlueprintAssignable, Category = "1984|Narrative")
	FOnActChanged OnActChanged;

	// ---------------------------------------------------------------
	// SurveillanceSystem callback
	// ---------------------------------------------------------------

	/** Bound to USurveillanceSystem::OnSuspicionThresholdChanged */
	UFUNCTION()
	void OnSuspicionLevelChanged(float NewLevel, float OldLevel);

protected:
	/** Surveillance intensity multiplier per act (higher = NPCs more alert) */
	float GetSurveillanceIntensityForAct(ENarrativeAct Act) const;

	/** Check suspicion-based narrative triggers */
	void EvaluateNarrativeTriggers();
};
