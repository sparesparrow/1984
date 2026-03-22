#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DoublethinkWidget.generated.h"

/**
 * UDoublethinkWidget
 *
 * VR widget for the Doublethink mechanic — a unique bimanual interaction
 * where the player holds two contradictory beliefs simultaneously.
 * Left hand = Party truth, right hand = historical truth.
 * The player must reconcile or choose between them.
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

	/** Set the cognitive dissonance score for this challenge */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void SetDissonanceScore(float Score);

	/** Player accepts Party truth (conformity) */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void AcceptPartyTruth();

	/** Player accepts historical truth (resistance) */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void AcceptHistoricalTruth();

	/** Player holds both truths simultaneously (true doublethink) */
	UFUNCTION(BlueprintCallable, Category = "1984|Doublethink")
	void AcceptBothTruths();

	/** Delegate when player makes a doublethink choice */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoublethinkChoice, FString, ChosenTruth, float, DissonanceScore);

	UPROPERTY(BlueprintAssignable, Category = "1984|Doublethink")
	FOnDoublethinkChoice OnDoublethinkChoice;

protected:
	/** The Party's version of events */
	UPROPERTY(BlueprintReadOnly, Category = "1984|Doublethink")
	FString PartyTruth;

	/** The actual historical truth */
	UPROPERTY(BlueprintReadOnly, Category = "1984|Doublethink")
	FString HistoricalTruth;

	/** Cognitive dissonance score (0.0 = easy, 1.0 = deeply contradictory) */
	UPROPERTY(BlueprintReadOnly, Category = "1984|Doublethink")
	float DissonanceScore;
};
