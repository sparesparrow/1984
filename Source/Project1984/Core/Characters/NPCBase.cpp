#include "NPCBase.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Characters/WinstonCharacter.h"
#include "Project1984/Core/Utils/VisionConeUtils.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Loyalty               = ENPCLoyalty::OuterParty;
	DetectionRadius       = 500.0f;
	bIsSuspicious         = false;
	SurveillanceSystem    = nullptr;
	CurrentWaypointIndex  = 0;
	WaypointAcceptRadius  = 50.0f;
	bWaypointMoveActive   = false;
	ReportCooldownRemaining = 0.0f;
	PlayerObservationTime = 0.0f;
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();

	// Register as a surveillance source so SurveillanceSystem can count active watchers.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
		if (SurveillanceSystem)
		{
			SurveillanceSystem->RegisterSurveillanceActor(this);
		}
	}

	// Tune detection radius by loyalty classification — ThoughtPolice are hyper-vigilant.
	switch (Loyalty)
	{
	case ENPCLoyalty::ThoughtPolice:  DetectionRadius = 800.0f; break;
	case ENPCLoyalty::PartyFaithful:  DetectionRadius = 600.0f; break;
	case ENPCLoyalty::OuterParty:     DetectionRadius = 400.0f; break;
	case ENPCLoyalty::Prole:          DetectionRadius = 200.0f; break;
	}

	UE_LOG(LogTemp, Log, TEXT("NPCBase '%s': Initialized. Loyalty=%d DetectionRadius=%.0f"),
		*GetName(), static_cast<int32>(Loyalty), DetectionRadius);
}

void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tick report cooldown
	if (ReportCooldownRemaining > 0.0f)
	{
		ReportCooldownRemaining -= DeltaTime;
	}

	if (CanSeePlayer())
	{
		EvaluatePlayerBehavior(DeltaTime);
	}
	else
	{
		PlayerObservationTime = 0.0f;
	}

	ExecutePatrol();
}

void ANPCBase::ReportPlayer()
{
	if (ReportCooldownRemaining > 0.0f || !SurveillanceSystem) return;

	const float Weight = GetReportWeight();
	SurveillanceSystem->ReportIncident(ESuspicionEvent::CitizenReport, Weight);

	bIsSuspicious            = true;
	ReportCooldownRemaining  = 30.0f; // 30-second report cooldown per NPC

	UE_LOG(LogTemp, Warning,
		TEXT("NPCBase '%s' (Loyalty=%d): Reported player — CitizenReport weight=%.2f"),
		*GetName(), static_cast<int32>(Loyalty), Weight);
}

bool ANPCBase::CanSeePlayer() const
{
	// Proles rarely notice or care about Party rules.
	if (Loyalty == ENPCLoyalty::Prole) return false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return false;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return false;

	// NPCs use a wide 180° forward half-space (90° half-angle) — alert but not camera-tight.
	return UVisionConeUtils::IsActorInVisionCone(
		this, PlayerPawn, /*ConeHalfAngleDeg=*/90.0f, DetectionRadius, /*bRequireLineOfSight=*/true);
}

void ANPCBase::ExecutePatrol()
{
	if (PatrolRoute.Num() == 0) return;

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC) return;

	if (bWaypointMoveActive)
	{
		const FVector Target   = PatrolRoute[CurrentWaypointIndex];
		const float   DistLeft = FVector::Dist(GetActorLocation(), Target);

		if (DistLeft <= WaypointAcceptRadius)
		{
			bWaypointMoveActive = false;
			AIC->StopMovement();

			// Pause 2 seconds at each waypoint before moving on.
			GetWorldTimerManager().SetTimer(
				WaypointPauseTimer, this, &ANPCBase::AdvanceWaypoint, 2.0f, /*bLoop=*/false);
		}
	}
	else if (!GetWorldTimerManager().IsTimerActive(WaypointPauseTimer))
	{
		AIC->MoveToLocation(PatrolRoute[CurrentWaypointIndex], WaypointAcceptRadius);
		bWaypointMoveActive = true;
	}
}

void ANPCBase::AdvanceWaypoint()
{
	CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolRoute.Num();
	bWaypointMoveActive  = false;
}

void ANPCBase::EvaluatePlayerBehavior(float DeltaTime)
{
	PlayerObservationTime += DeltaTime;

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()
		? GetWorld()->GetFirstPlayerController()->GetPawn()
		: nullptr;

	AWinstonCharacter* Winston = Cast<AWinstonCharacter>(PlayerPawn);
	if (!Winston) return;

	bool bPlayerSuspicious = false;

	// Writing the diary is an immediately observable suspicious act.
	if (Winston->bIsWritingDiary)
	{
		bPlayerSuspicious = true;
	}

	// Being in a restricted area is suspicious (zone actors tag the pawn).
	if (PlayerPawn->ActorHasTag(FName("InRestrictedZone")))
	{
		bPlayerSuspicious = true;
	}

	if (bPlayerSuspicious)
	{
		const float Threshold = GetReportThreshold();
		if (PlayerObservationTime >= Threshold)
		{
			ReportPlayer();
			PlayerObservationTime = 0.0f;
		}
	}
	else
	{
		PlayerObservationTime = FMath::Max(0.0f, PlayerObservationTime - DeltaTime * 0.5f);
	}
}

float ANPCBase::GetReportThreshold() const
{
	switch (Loyalty)
	{
	case ENPCLoyalty::ThoughtPolice:  return 0.5f;
	case ENPCLoyalty::PartyFaithful:  return 1.0f;
	case ENPCLoyalty::OuterParty:     return 3.0f;
	case ENPCLoyalty::Prole:          return 10.0f;
	}
	return 5.0f;
}

float ANPCBase::GetReportWeight() const
{
	switch (Loyalty)
	{
	case ENPCLoyalty::ThoughtPolice:  return 0.30f;
	case ENPCLoyalty::PartyFaithful:  return 0.20f;
	case ENPCLoyalty::OuterParty:     return 0.10f;
	case ENPCLoyalty::Prole:          return 0.05f;
	}
	return 0.10f;
}
