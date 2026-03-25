#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PropagandaHUD.generated.h"

class USurveillanceSystem;
class UNarrativeManager;

/**
 * APropagandaHUD
 *
 * World-space VR HUD for the 1984 simulation. All UI is diegetic —
 * information is presented through in-world objects (Party posters,
 * telescreen overlays, work terminals) rather than floating VR panels.
 * This reinforces the oppressive atmosphere and avoids breaking presence.
 *
 * The C++ layer manages state, data selection, and delegate wiring.
 * Blueprint subclass drives actual UWidgetComponent / UTextRenderComponent
 * visuals by listening to the delegates and reading the public properties.
 */
UCLASS()
class PROJECT1984_API APropagandaHUD : public AHUD
{
	GENERATED_BODY()

public:
	APropagandaHUD();

	virtual void BeginPlay() override;

	// ---------------------------------------------------------------
	// Public API — called by game systems and Blueprint
	// ---------------------------------------------------------------

	/** Show a Party slogan on nearby world-space widgets */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void DisplayPartySlogan(const FString& Slogan);

	/** Show suspicion indicator (diegetic — e.g., poster eyes following player) */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void UpdateSuspicionIndicator(float SuspicionLevel);

	/** Display a Newspeak translation prompt on the work terminal */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void ShowNewspeakChallenge(const FString& OldspeakPhrase, const FString& ExpectedNewspeak);

	/** Hide the Newspeak challenge */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void HideNewspeakChallenge();

	/** Show the educational debrief screen (Act V) */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void ShowDebriefScreen(const FString& EndingType, const FString& DecisionLogJSON);

	// ---------------------------------------------------------------
	// State readable by Blueprint widget actors
	// ---------------------------------------------------------------

	/** Currently active slogan text */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|UI")
	FString ActiveSlogan;

	/** Whether a Newspeak challenge is currently displayed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|UI")
	bool bNewspeakChallengeActive;

	/** Oldspeak phrase for the current Newspeak challenge */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|UI")
	FString NewspeakChallengeOldspeak;

	/** Expected Newspeak answer (hidden from player until reveal) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|UI")
	FString NewspeakChallengeExpected;

	// ---------------------------------------------------------------
	// Delegates — Blueprint widget actors bind to these
	// ---------------------------------------------------------------

	/** Fired when the active slogan changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSloganChanged, const FString&, NewSlogan);
	UPROPERTY(BlueprintAssignable, Category = "1984|UI")
	FOnSloganChanged OnSloganChanged;

	/** Fired when the diegetic suspicion state changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuspicionStateChanged, float, SuspicionLevel);
	UPROPERTY(BlueprintAssignable, Category = "1984|UI")
	FOnSuspicionStateChanged OnSuspicionStateChanged;

	/** Fired when the debrief screen should be shown */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowDebrief, const FString&, EndingType, const FString&, DebriefText);
	UPROPERTY(BlueprintAssignable, Category = "1984|UI")
	FOnShowDebrief OnShowDebrief;

protected:
	/** Cached subsystem references */
	USurveillanceSystem* SurveillanceSystem;
	UNarrativeManager*   NarrativeManager;

	/** Slogan rotation index */
	int32 SloganIndex;

	/** Timer handle for automatic slogan rotation */
	FTimerHandle SloganRotationTimer;

	/** Callback for slogan rotation tick */
	void RotateSloganTick();

	/** Update world-space propaganda content based on current act */
	void RefreshPropagandaContent();

	/** Bound to SurveillanceSystem::OnSuspicionThresholdChanged */
	UFUNCTION()
	void OnSuspicionThresholdChanged(float NewLevel, float OldLevel);
};
