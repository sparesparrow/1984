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

	/** Current narrative act */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	ENarrativeAct CurrentAct;

	/** Current scene within the act */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	FString CurrentScene;

	/** Advance to the next narrative act */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	void AdvanceAct();

	/** Transition to a specific scene within the current act */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	void TransitionToScene(const FString& SceneID);

	/** Check if narrative conditions are met to advance */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Narrative")
	bool CanAdvanceAct() const;

	/** Get the ending type based on player history */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Narrative")
	FString DetermineEnding() const;

	/** Delegate for act transitions */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActChanged, ENarrativeAct, NewAct, ENarrativeAct, PreviousAct);

	UPROPERTY(BlueprintAssignable, Category = "1984|Narrative")
	FOnActChanged OnActChanged;

protected:
	/** Check suspicion-based narrative triggers */
	void EvaluateNarrativeTriggers();
};
