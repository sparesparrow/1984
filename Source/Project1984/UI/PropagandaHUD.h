#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PropagandaHUD.generated.h"

/**
 * APropagandaHUD
 *
 * World-space VR HUD for the 1984 simulation. All UI is diegetic —
 * information is presented through in-world objects (Party posters,
 * telescreen overlays, work terminals) rather than floating VR panels.
 * This reinforces the oppressive atmosphere and avoids breaking presence.
 */
UCLASS()
class PROJECT1984_API APropagandaHUD : public AHUD
{
	GENERATED_BODY()

public:
	APropagandaHUD();

	virtual void BeginPlay() override;

	/** Show a Party slogan on nearby world-space widgets */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void DisplayPartySlogan(const FString& Slogan);

	/** Show suspicion indicator (diegetic — e.g., poster eyes following player) */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void UpdateSuspicionIndicator(float SuspicionLevel);

	/** Display a Newspeak translation prompt */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void ShowNewspeakChallenge(const FString& OldspeakPhrase, const FString& ExpectedNewspeak);

	/** Hide the Newspeak challenge */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void HideNewspeakChallenge();

	/** Show the educational debrief screen (Act V) */
	UFUNCTION(BlueprintCallable, Category = "1984|UI")
	void ShowDebriefScreen(const FString& EndingType, const FString& DecisionLogJSON);

protected:
	/** Update world-space poster widgets with current propaganda content */
	void RefreshPropagandaContent();
};
