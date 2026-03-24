#include "ThoughtPoliceAI.h"
#include "SurveillanceSystem.h"
#include "NarrativeManager.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AThoughtPoliceAI::AThoughtPoliceAI()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentState             = EPatrolState::Patrolling;
	VisionConeAngle          = 45.0f;
	DetectionRange           = 1500.0f;
	DetectionTime            = 2.0f;
	ArrestRange              = 120.0f;
	WaypointAcceptRadius     = 50.0f;
	CurrentWaypointIndex     = 0;
	ContinuousDetectionTime  = 0.0f;
	PursuitTimeWithoutSight  = 0.0f;
	ObservationElapsedTime   = 0.0f;
	bWaypointMoveActive      = false;
	SurveillanceSystem       = nullptr;
	NarrativeManager         = nullptr;
}

void AThoughtPoliceAI::BeginPlay()
{
	Super::BeginPlay();

	// Resolve subsystem references
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
		NarrativeManager   = GI->GetSubsystem<UNarrativeManager>();
	}

	// Register with SurveillanceSystem as a surveillance source
	if (SurveillanceSystem && GetPawn())
	{
		SurveillanceSystem->RegisterSurveillanceActor(GetPawn());
	}

	// Patrol route is set by level designers via SetPatrolRoute() or a DataAsset
	// wired in the owning pawn Blueprint.
	UE_LOG(LogTemp, Log, TEXT("ThoughtPoliceAI: Initialized. PatrolWaypoints=%d"), PatrolRoute.Num());
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
				CurrentState            = EPatrolState::Observing;
				ObservationElapsedTime  = 0.0f;
				StopMovement();
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
		// Arrest is handled via timer set in InitiateArrest; no per-tick action needed
		break;

	case EPatrolState::Undercover:
		// Behave as a normal citizen — use patrol logic until suspicion threshold reveals identity
		ExecutePatrol();
		if (SurveillanceSystem && SurveillanceSystem->GetSuspicionLevel() >= 0.6f)
		{
			UE_LOG(LogTemp, Log, TEXT("ThoughtPoliceAI: Undercover agent revealed at suspicion 0.6+"));
			CurrentState = EPatrolState::Patrolling;
		}
		break;
	}
}

void AThoughtPoliceAI::SetPatrolRoute(const TArray<FVector>& PatrolPoints)
{
	PatrolRoute          = PatrolPoints;
	CurrentWaypointIndex = 0;
	bWaypointMoveActive  = false;
}

void AThoughtPoliceAI::BeginPursuit()
{
	CurrentState            = EPatrolState::Pursuing;
	PursuitTimeWithoutSight = 0.0f;

	// Report ThoughtPoliceDetection event to SurveillanceSystem (+0.35)
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::ThoughtPoliceDetection,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::ThoughtPoliceDetection));
	}

	UE_LOG(LogTemp, Warning, TEXT("ThoughtPoliceAI: Pursuit started."));
}

void AThoughtPoliceAI::InitiateArrest()
{
	CurrentState = EPatrolState::Arresting;
	StopMovement();

	// Report final detection event
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::ThoughtPoliceDetection,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::ThoughtPoliceDetection));
	}

	// Notify NarrativeManager to transition to Act IV (Room 101)
	if (NarrativeManager)
	{
		NarrativeManager->ForceAdvanceToCapture();
	}

	// Disable player movement via the player pawn
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			PlayerPawn->DisableInput(PC);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ThoughtPoliceAI: Arrest initiated — Act IV triggered."));
}

bool AThoughtPoliceAI::IsPlayerInVisionCone() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		return false;
	}

	const FVector MyLocation     = ControlledPawn->GetActorLocation();
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const FVector ToPlayer       = (PlayerLocation - MyLocation);
	const float   Distance       = ToPlayer.Size();

	// Distance check
	if (Distance > DetectionRange)
	{
		return false;
	}

	// Angle check — dot product against controlled pawn's forward vector
	const FVector ToPlayerNorm = ToPlayer.GetSafeNormal();
	const FVector MyForward    = ControlledPawn->GetActorForwardVector();
	const float   CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(VisionConeAngle * 0.5f));
	if (FVector::DotProduct(MyForward, ToPlayerNorm) < CosHalfAngle)
	{
		return false;
	}

	// Line-of-sight trace — ensure no geometry obstructs the view
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledPawn);
	Params.AddIgnoredActor(PlayerPawn);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		MyLocation + FVector(0.f, 0.f, 60.f),   // eye height offset
		PlayerLocation + FVector(0.f, 0.f, 60.f),
		ECC_Visibility,
		Params);

	return !bBlocked;
}

void AThoughtPoliceAI::ExecutePatrol()
{
	if (PatrolRoute.Num() == 0)
	{
		return;
	}

	// If a waypoint move is already in flight wait for arrival (handled by timer)
	if (bWaypointMoveActive)
	{
		const FVector Target        = PatrolRoute[CurrentWaypointIndex];
		const FVector MyLoc         = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
		const float   DistToTarget  = FVector::Dist(MyLoc, Target);

		if (DistToTarget <= WaypointAcceptRadius)
		{
			// Reached waypoint — pause briefly then advance
			bWaypointMoveActive = false;
			StopMovement();
			GetWorldTimerManager().SetTimer(
				WaypointPauseHandle, this,
				&AThoughtPoliceAI::AdvanceToNextWaypoint,
				1.5f, /*bLoop=*/false);
		}
	}
	else if (!GetWorldTimerManager().IsTimerActive(WaypointPauseHandle))
	{
		// Start moving to the next waypoint
		MoveToLocation(PatrolRoute[CurrentWaypointIndex], WaypointAcceptRadius);
		bWaypointMoveActive = true;
	}
}

void AThoughtPoliceAI::AdvanceToNextWaypoint()
{
	CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolRoute.Num();
	bWaypointMoveActive  = false;
}

void AThoughtPoliceAI::ExecuteObservation()
{
	APawn* PlayerPawn = nullptr;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PlayerPawn = PC->GetPawn();
	}

	// Face toward player
	if (PlayerPawn && GetPawn())
	{
		const FVector ToPlayer =
			(PlayerPawn->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
		const FRotator LookAt = FRotationMatrix::MakeFromX(ToPlayer).Rotator();
		GetPawn()->SetActorRotation(FMath::RInterpTo(
			GetPawn()->GetActorRotation(), LookAt,
			GetWorld()->GetDeltaSeconds(), 5.0f));
	}

	ObservationElapsedTime += GetWorld()->GetDeltaSeconds();

	// Report passive telescreen-style observation event every 3 seconds
	if (SurveillanceSystem && FMath::Fmod(ObservationElapsedTime, 3.0f) < GetWorld()->GetDeltaSeconds())
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::TelescreenObservation,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::TelescreenObservation));
	}

	// After 4 seconds of observation escalate to pursuit;
	// return to patrol if player leaves the vision cone
	if (ObservationElapsedTime >= 4.0f)
	{
		BeginPursuit();
	}
	else if (!IsPlayerInVisionCone())
	{
		CurrentState           = EPatrolState::Patrolling;
		ContinuousDetectionTime = 0.0f;
		ObservationElapsedTime  = 0.0f;
	}
}

void AThoughtPoliceAI::ExecutePursuit()
{
	APawn* PlayerPawn = nullptr;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PlayerPawn = PC->GetPawn();
	}

	if (!PlayerPawn || !GetPawn())
	{
		return;
	}

	const float DistToPlayer =
		FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

	// Initiate arrest when close enough
	if (DistToPlayer <= ArrestRange)
	{
		InitiateArrest();
		return;
	}

	// Move directly toward player
	MoveToActor(PlayerPawn, ArrestRange * 0.5f);

	if (IsPlayerInVisionCone())
	{
		PursuitTimeWithoutSight = 0.0f;
	}
	else
	{
		PursuitTimeWithoutSight += GetWorld()->GetDeltaSeconds();
		if (PursuitTimeWithoutSight >= 10.0f)
		{
			// Player escaped — return to patrol
			UE_LOG(LogTemp, Log, TEXT("ThoughtPoliceAI: Player escaped — resuming patrol."));
			CurrentState            = EPatrolState::Patrolling;
			ContinuousDetectionTime = 0.0f;
			PursuitTimeWithoutSight = 0.0f;
			bWaypointMoveActive     = false;
		}
	}
}
