#include "ThoughtPoliceAI.h"
#include "SurveillanceSystem.h"
#include "NarrativeManager.h"
#include "NavigationSystem.h"
#include "Project1984/Core/Utils/VisionConeUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

AThoughtPoliceAI::AThoughtPoliceAI()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentState             = EPatrolState::Patrolling;
	VisionConeAngle          = 45.0f;
	DetectionRange           = 1500.0f;
	DetectionTime            = 2.0f;
	ArrestRange              = 150.0f;
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
	// Pawn registration with SurveillanceSystem is deferred to OnPossess
	// so GetPawn() is guaranteed non-null.
}

void AThoughtPoliceAI::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
			NarrativeManager   = GI->GetSubsystem<UNarrativeManager>();

			if (SurveillanceSystem)
			{
				SurveillanceSystem->RegisterSurveillanceActor(InPawn);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ThoughtPoliceAI: Possessing '%s'. PatrolWaypoints=%d"),
		*InPawn->GetName(), PatrolRoute.Num());
}

void AThoughtPoliceAI::OnUnPossess()
{
	if (APawn* OldPawn = GetPawn())
	{
		if (SurveillanceSystem)
		{
			SurveillanceSystem->UnregisterSurveillanceActor(OldPawn);
		}
	}
	Super::OnUnPossess();
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
				CurrentState           = EPatrolState::Observing;
				ObservationElapsedTime = 0.0f;
				ContinuousDetectionTime = 0.0f;
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
		// Static — arrest animation and act transition handled in InitiateArrest().
		break;

	case EPatrolState::Undercover:
		// Blend in until global suspicion is high enough to reveal.
		ExecutePatrol();
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
			{
				if (SS->GlobalSuspicion >= 0.6f && IsPlayerInVisionCone())
				{
					UE_LOG(LogTemp, Log, TEXT("ThoughtPoliceAI: Undercover agent revealed at suspicion 0.6+"));
					BeginPursuit();
				}
			}
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

	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::ThoughtPoliceDetection,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::ThoughtPoliceDetection));
	}

	UE_LOG(LogTemp, Warning, TEXT("ThoughtPoliceAI: Pursuit started (+0.35 suspicion)."));
}

void AThoughtPoliceAI::InitiateArrest()
{
	CurrentState = EPatrolState::Arresting;
	StopMovement();

	// Final detection event.
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::ThoughtPoliceDetection,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::ThoughtPoliceDetection));
	}

	// Force narrative to Act IV (Capture & Conditioning / Room 101).
	if (NarrativeManager)
	{
		while (NarrativeManager->CurrentAct < ENarrativeAct::CaptureConditioning)
		{
			NarrativeManager->AdvanceAct();
		}
	}

	// Disable player movement.
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
	if (!ControlledPawn) return false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return false;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return false;

	return UVisionConeUtils::IsActorInVisionCone(
		ControlledPawn, PlayerPawn, VisionConeAngle, DetectionRange, /*bRequireLineOfSight=*/true);
}

void AThoughtPoliceAI::ExecutePatrol()
{
	if (PatrolRoute.Num() == 0) return;

	if (bWaypointMoveActive)
	{
		const FVector Target       = PatrolRoute[CurrentWaypointIndex];
		const FVector MyLoc        = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
		const float   DistToTarget = FVector::Dist(MyLoc, Target);

		if (DistToTarget <= WaypointAcceptRadius)
		{
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
	StopMovement();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn*             PP = PC ? PC->GetPawn() : nullptr;

	// Face toward player while assessing.
	if (APawn* ControlledPawn = GetPawn())
	{
		if (PP)
		{
			const FVector Dir     = (PP->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
			const FRotator LookAt = FRotationMatrix::MakeFromX(Dir).Rotator();
			ControlledPawn->SetActorRotation(FMath::RInterpTo(
				ControlledPawn->GetActorRotation(), LookAt, GetWorld()->GetDeltaSeconds(), 5.0f));
		}
	}

	ObservationElapsedTime += GetWorld()->GetDeltaSeconds();

	// Report passive observation event every 3 seconds.
	if (SurveillanceSystem && FMath::Fmod(ObservationElapsedTime, 3.0f) < GetWorld()->GetDeltaSeconds())
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::TelescreenObservation,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::TelescreenObservation));
	}

	// If player left view, resume patrol.
	if (!IsPlayerInVisionCone())
	{
		CurrentState            = EPatrolState::Patrolling;
		ContinuousDetectionTime = 0.0f;
		ObservationElapsedTime  = 0.0f;
		return;
	}

	// Escalate to pursuit when suspicion passes the investigation threshold.
	if (ObservationElapsedTime >= 4.0f)
	{
		BeginPursuit();
		return;
	}

	if (SurveillanceSystem && SurveillanceSystem->GlobalSuspicion >= 0.6f)
	{
		BeginPursuit();
	}
}

void AThoughtPoliceAI::ExecutePursuit()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	APawn* PlayerPawn    = PC->GetPawn();
	APawn* ControlledPawn = GetPawn();
	if (!PlayerPawn || !ControlledPawn) return;

	const float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

	if (Dist <= ArrestRange)
	{
		InitiateArrest();
		return;
	}

	MoveToActor(PlayerPawn, ArrestRange * 0.5f, /*bStopOnOverlap=*/true);

	if (!IsPlayerInVisionCone())
	{
		PursuitTimeWithoutSight += GetWorld()->GetDeltaSeconds();
		if (PursuitTimeWithoutSight >= 10.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("ThoughtPoliceAI: Player escaped — resuming patrol."));
			CurrentState            = EPatrolState::Patrolling;
			ContinuousDetectionTime = 0.0f;
			PursuitTimeWithoutSight = 0.0f;
			bWaypointMoveActive     = false;
		}
	}
	else
	{
		PursuitTimeWithoutSight = 0.0f;
	}
}
