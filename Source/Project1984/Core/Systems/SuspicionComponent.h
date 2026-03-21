#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurveillanceSystem.h"
#include "SuspicionComponent.generated.h"

/**
 * USuspicionComponent
 *
 * Attached to the player character (Winston) and surveillance-aware NPCs.
 * Tracks per-actor suspicion contributions and communicates with the
 * global SurveillanceSystem.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT1984_API USuspicionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USuspicionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** This actor's personal suspicion contribution */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Suspicion")
	float PersonalSuspicion;

	/** Rate at which suspicion decays over time when not committing suspicious acts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|Suspicion")
	float SuspicionDecayRate;

	/** Whether this actor is currently under direct surveillance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Suspicion")
	bool bUnderSurveillance;

	/** Report a suspicion event from this actor */
	UFUNCTION(BlueprintCallable, Category = "1984|Suspicion")
	void ReportSuspicionEvent(ESuspicionEvent Event);

	/** Report a suspicion event with a custom weight override */
	UFUNCTION(BlueprintCallable, Category = "1984|Suspicion")
	void ReportSuspicionEventWithWeight(ESuspicionEvent Event, float CustomWeight);

protected:
	/** Apply natural suspicion decay over time */
	void ApplySuspicionDecay(float DeltaTime);
};
