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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

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

	/** Distance within which the arrest is initiated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|ThoughtPolice")
	float ArrestRange;

	/** Patrol waypoint accept radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|ThoughtPolice")
	float WaypointAcceptRadius;

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

	/** Time spent in Pursuing state without line-of-sight (resets patrol after timeout) */
	float PursuitTimeWithoutSight;

	/** Time spent in Observing state */
	float ObservationElapsedTime;

	/** True when a waypoint move command is in flight */
	bool bWaypointMoveActive;

	/** Cached subsystems — resolved in BeginPlay */
	class USurveillanceSystem* SurveillanceSystem;
	class UNarrativeManager*   NarrativeManager;

	/** Execute patrol behavior */
	void ExecutePatrol();

	/** Execute observation behavior */
	void ExecuteObservation();

	/** Execute pursuit behavior */
	void ExecutePursuit();

	/** Timer handle for patrol waypoint pause */
	FTimerHandle WaypointPauseHandle;

	/** Called after the waypoint pause expires to advance to the next waypoint */
	void AdvanceToNextWaypoint();
};
