#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Project1984GameState.generated.h"

/**
 * AProject1984GameState
 *
 * Tracks persistent world state: global suspicion level, Party alert status,
 * completed narrative events, and the player's decision history for
 * determining which of the multiple endings they reach.
 */
UCLASS()
class PROJECT1984_API AProject1984GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AProject1984GameState();

	/** Global suspicion level (0.0 = unsuspected, 1.0 = imminent capture) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Suspicion")
	float GlobalSuspicionLevel;

	/** Number of times the player has been observed committing thoughtcrime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Suspicion")
	int32 ThoughtcrimeCount;

	/** Whether the player has made contact with Julia */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	bool bJuliaContactMade;

	/** Whether the player has met O'Brien */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	bool bOBrienMeetingComplete;

	/** Record a narrative event for the decision log */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	void RecordDecision(const FString& DecisionID, const FString& Choice);

	/** Export decision log as JSON (for educational debrief) */
	UFUNCTION(BlueprintCallable, Category = "1984|Narrative")
	FString ExportDecisionLogJSON() const;

protected:
	/** History of player decisions for multiple-ending determination */
	UPROPERTY()
	TMap<FString, FString> DecisionLog;
};
