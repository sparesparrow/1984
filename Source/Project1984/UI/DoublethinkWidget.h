#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DoublethinkWidget.generated.h"

class USurveillanceSystem;
class UNarrativeManager;

/**
 * UDoublethinkWidget
 *
 * VR widget for the Doublethink mechanic — a unique bimanual interaction
 * where the player holds two contradictory beliefs simultaneously.
 * Left hand = Party truth, right hand = historical truth.
 * The player must reconcile or choose between them.
 *
 * C++ layer: data, subsystem reporting, choice consequences.
 * Blueprint subclass: world-space bimanual visual layout and animations.
 */
UCLASS()
class PROJECT1984_API UDoublethinkWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the Party's version of truth (left hand) */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void SetPartyTruth(const FString& PartyVersion);

	/** Set the historical/real truth (right hand) */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void SetHistoricalTruth(const FString& HistoricalVersion);

	/**
	 * Set cognitive dissonance score (0.0 = trivial contradiction, 1.0 = maximal).
	 * High dissonance (> 0.7) triggers a FacialExpression suspicion event because
	 * the player's involuntary reactions betray inner conflict to telescreens.
	 */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void SetDissonanceScore(float Score);

	/** Player accepts Party truth — conformity, slight suspicion reduction */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void AcceptPartyTruth();

	/** Player accepts historical truth — resistance, suspicion increase */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void AcceptHistoricalTruth();

	/** Player holds both truths simultaneously — true doublethink, no net suspicion change */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void AcceptBothTruths();

	/** Reset the widget for reuse in a new challenge */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void ResetChoice();

	// ---------------------------------------------------------------
	// Delegates
	// ---------------------------------------------------------------

	/**
	 * Broadcast when the player commits to a choice.
	 * ChosenTruth: "Party" | "Historical" | "Doublethink"
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoublethinkChoice, FString, ChosenTruth, float, DissonanceScore);

	UPROPERTY(BlueprintAssignable, Category = "1984|Doublethink")
	FOnDoublethinkChoice OnDoublethinkChoice;

	// ---------------------------------------------------------------
	// State readable by Blueprint
	// ---------------------------------------------------------------

	/** The Party's version of the contested fact */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Doublethink")
	FString PartyTruth;

	/** The actual historical fact */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Doublethink")
	FString HistoricalTruth;

	/** Cognitive dissonance score (0.0–1.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Doublethink")
	float DissonanceScore;

	/** Whether the player has already committed a choice this session */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Doublethink")
	bool bChoiceMade;

protected:
	virtual void NativeConstruct() override;

	/** Cached subsystem references resolved in NativeConstruct */
	USurveillanceSystem* SurveillanceSystem;
	UNarrativeManager*   NarrativeManager;
};
