#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TelescreenComponent.generated.h"

class USurveillanceSystem;
class USurveillanceAudioManager;

/**
 * ETelescreenState
 *
 * Telescreen operational states.
 */
UENUM(BlueprintType)
enum class ETelescreenState : uint8
{
	/** Normal broadcast — propaganda loop */
	Broadcasting	UMETA(DisplayName = "Broadcasting"),
	/** Directly addressing the player */
	Addressing		UMETA(DisplayName = "Addressing Player"),
	/** Two Minutes Hate broadcast */
	TwoMinutesHate	UMETA(DisplayName = "Two Minutes Hate"),
	/** Disabled/obscured by player (risky) */
	Obscured		UMETA(DisplayName = "Obscured"),
	/** Off — power failure or restricted area */
	Off				UMETA(DisplayName = "Off")
};

/**
 * UTelescreenComponent
 *
 * Every room in Airstrip One has telescreens. They are both output
 * (propaganda broadcast) and input (surveillance). The telescreen has
 * a vision cone that detects the player and triggers suspicion events.
 * Players can partially obscure telescreens (risk/reward mechanic).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT1984_API UTelescreenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTelescreenComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Current telescreen state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Telescreen")
	ETelescreenState State;

	/** Vision cone half-angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|Telescreen")
	float VisionConeAngle;

	/** Maximum surveillance range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|Telescreen")
	float SurveillanceRange;

	/** Whether this telescreen is currently observing the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Telescreen")
	bool bPlayerInView;

	/** Seconds between TelescreenObservation reports when player is in view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|Telescreen")
	float ObservationReportInterval;

	/** Begin Two Minutes Hate broadcast */
	UFUNCTION(BlueprintCallable, Category = "1984|Telescreen")
	void StartTwoMinutesHate();

	/** End Two Minutes Hate broadcast */
	UFUNCTION(BlueprintCallable, Category = "1984|Telescreen")
	void EndTwoMinutesHate();

	/** Attempt to obscure the telescreen (player action) */
	UFUNCTION(BlueprintCallable, Category = "1984|Telescreen")
	void ObscureTelescreen();

	/** Check if the player is within the telescreen's vision cone */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Telescreen")
	bool IsPlayerVisible() const;

protected:
	/** Cached subsystems */
	USurveillanceSystem*      SurveillanceSystem;
	USurveillanceAudioManager* AudioManager;

	/** Time since last TelescreenObservation report */
	float ObservationTimer;

	/** Time spent in Addressing state before returning to Broadcasting */
	float AddressingTimer;

	/** Whether the player participated in the Two Minutes Hate (fist/shout gesture) */
	bool bParticipatedInHate;

	/** Index into propaganda content rotation */
	int32 PropagandaIndex;

	/** Timer handle for auto-restoring obscured telescreen */
	FTimerHandle RestoreTimerHandle;

	/** Called by timer to restore the telescreen after obscuring */
	void RestoreFromObscured();

	/** Check surveillance coverage and report to SurveillanceSystem */
	void UpdateSurveillance(float DeltaTime);

	/** Update propaganda video playback */
	void UpdateBroadcast(float DeltaTime);
};
