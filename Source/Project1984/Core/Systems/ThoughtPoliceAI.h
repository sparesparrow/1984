#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ThoughtPoliceAI.generated.h"

/**
 * EPatrolState
 *
 * Thought Police behavioral states.
 */
UENUM(BlueprintType)
enum class EPatrolState : uint8
{
	/** Standard patrol route */
	Patrolling		UMETA(DisplayName = "Patrolling"),
	/** Observing a suspicious individual */
	Observing		UMETA(DisplayName = "Observing"),
	/** Actively pursuing a target */
	Pursuing		UMETA(DisplayName = "Pursuing"),
	/** Arresting the target */
	Arresting		UMETA(DisplayName = "Arresting"),
	/** Undercover — disguised as ordinary citizen */
	Undercover		UMETA(DisplayName = "Undercover")
};

/**
 * AThoughtPoliceAI
 *
 * AI controller for Thought Police agents. They patrol routes,
 * have vision cones that trigger suspicion events, and can
 * escalate to pursuit and arrest at high suspicion levels.
 */
UCLASS()
class PROJECT1984_API AThoughtPoliceAI : public AAIController
{
	GENERATED_BODY()

public:
	AThoughtPoliceAI();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Current behavioral state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|ThoughtPolice")
	EPatrolState CurrentState;

	/** Vision cone half-angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|ThoughtPolice")
	float VisionConeAngle;

	/** Maximum detection distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|ThoughtPolice")
	float DetectionRange;

	/** Time (seconds) player must be in vision cone before detection triggers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|ThoughtPolice")
	float DetectionTime;

	/** Set a new patrol route */
	UFUNCTION(BlueprintCallable, Category = "1984|ThoughtPolice")
	void SetPatrolRoute(const TArray<FVector>& PatrolPoints);

	/** Force transition to pursuit state */
	UFUNCTION(BlueprintCallable, Category = "1984|ThoughtPolice")
	void BeginPursuit();

	/** Initiate arrest sequence */
	UFUNCTION(BlueprintCallable, Category = "1984|ThoughtPolice")
	void InitiateArrest();

	/** Check if the player is within the vision cone */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|ThoughtPolice")
	bool IsPlayerInVisionCone() const;

protected:
	/** Patrol route waypoints */
	UPROPERTY()
	TArray<FVector> PatrolRoute;

	/** Current waypoint index */
	int32 CurrentWaypointIndex;

	/** Time player has been continuously detected */
	float ContinuousDetectionTime;

	/** Execute patrol behavior */
	void ExecutePatrol();

	/** Execute observation behavior */
	void ExecuteObservation();

	/** Execute pursuit behavior */
	void ExecutePursuit();
};
