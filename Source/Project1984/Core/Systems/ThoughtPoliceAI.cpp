#include "ThoughtPoliceAI.h"
#include "SurveillanceSystem.h"

AThoughtPoliceAI::AThoughtPoliceAI()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentState = EPatrolState::Patrolling;
	VisionConeAngle = 45.0f;
	DetectionRange = 1500.0f;
	DetectionTime = 2.0f;
	CurrentWaypointIndex = 0;
	ContinuousDetectionTime = 0.0f;
}

void AThoughtPoliceAI::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Register with SurveillanceSystem as a surveillance source
	// TODO: Load patrol route from level-placed spline or data asset
}

void AThoughtPoliceAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case EPatrolState::Patrolling:
		ExecutePatrol();
		if (IsPlayerInVisionCone())
		{
			ContinuousDetectionTime += DeltaTime;
			if (ContinuousDetectionTime >= DetectionTime)
			{
				CurrentState = EPatrolState::Observing;
			}
		}
		else
		{
			ContinuousDetectionTime = FMath::Max(0.0f, ContinuousDetectionTime - DeltaTime * 0.5f);
		}
		break;

	case EPatrolState::Observing:
		ExecuteObservation();
		break;

	case EPatrolState::Pursuing:
		ExecutePursuit();
		break;

	case EPatrolState::Arresting:
		// TODO: Play arrest animation and trigger Act IV transition
		break;

	case EPatrolState::Undercover:
		// TODO: Behave as normal citizen until suspicion threshold triggers reveal
		ExecutePatrol();
		break;
	}
}

void AThoughtPoliceAI::SetPatrolRoute(const TArray<FVector>& PatrolPoints)
{
	PatrolRoute = PatrolPoints;
	CurrentWaypointIndex = 0;
}

void AThoughtPoliceAI::BeginPursuit()
{
	CurrentState = EPatrolState::Pursuing;

	// TODO: Report ThoughtPoliceDetection event to SurveillanceSystem (+0.35)
	// TODO: Alert nearby Thought Police agents
	// TODO: Play pursuit audio cue
}

void AThoughtPoliceAI::InitiateArrest()
{
	CurrentState = EPatrolState::Arresting;

	// TODO: Trigger arrest cinematic
	// TODO: Notify NarrativeManager to transition to Act IV (Room 101)
	// TODO: Disable player movement
}

bool AThoughtPoliceAI::IsPlayerInVisionCone() const
{
	// TODO: Get player pawn location
	// TODO: Calculate angle between forward vector and direction to player
	// TODO: Check if angle < VisionConeAngle and distance < DetectionRange
	// TODO: Perform line-of-sight trace to ensure no obstructions
	return false;
}

void AThoughtPoliceAI::ExecutePatrol()
{
	// TODO: Navigate to current waypoint using AI navigation
	// TODO: When reached, advance to next waypoint
	// TODO: Pause briefly at each waypoint to "observe"
	if (PatrolRoute.Num() > 0)
	{
		// TODO: MoveToLocation(PatrolRoute[CurrentWaypointIndex]);
	}
}

void AThoughtPoliceAI::ExecuteObservation()
{
	// TODO: Stop movement, face player
	// TODO: Report TelescreenObservation event to SurveillanceSystem
	// TODO: After observation period, either return to patrol or escalate to pursuit
	// TODO: Escalation depends on global suspicion level
}

void AThoughtPoliceAI::ExecutePursuit()
{
	// TODO: Navigate directly toward player at increased speed
	// TODO: Call for backup (alert other Thought Police)
	// TODO: If within arrest range, initiate arrest
	// TODO: If player escapes line-of-sight for >10s, return to patrol
}
