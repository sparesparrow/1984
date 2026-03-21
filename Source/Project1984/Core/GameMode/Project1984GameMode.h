#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Project1984GameMode.generated.h"

/**
 * AProject1984GameMode
 *
 * Game mode for the 1984 VR simulation. Manages session-level game rules,
 * player spawning, and coordinates with the SurveillanceSystem and
 * NarrativeManager to drive the five-act story.
 */
UCLASS()
class PROJECT1984_API AProject1984GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProject1984GameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

	/** Get the current narrative act (1-5) */
	UFUNCTION(BlueprintCallable, Category = "1984|Game")
	int32 GetCurrentAct() const;

	/** Transition to the next narrative act */
	UFUNCTION(BlueprintCallable, Category = "1984|Game")
	void AdvanceAct();

	/** Check if the player has been captured by Thought Police */
	UFUNCTION(BlueprintCallable, Category = "1984|Game")
	bool IsPlayerCaptured() const;

protected:
	/** Current narrative act (1 = Ordinary Life, 5 = Resolution) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Narrative")
	int32 CurrentAct;

	/** Whether the player has been captured */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|State")
	bool bPlayerCaptured;

	/** Initialize core game subsystems */
	void InitializeSubsystems();
};
