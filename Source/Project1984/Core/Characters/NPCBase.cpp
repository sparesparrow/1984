#include "NPCBase.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Characters/WinstonCharacter.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

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

	// Resolve SurveillanceSystem
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
	}

	// Tune detection radius by loyalty classification
	switch (Loyalty)
	{
	case ENPCLoyalty::PartyFaithful:
		DetectionRadius = 700.0f;
		break;
	case ENPCLoyalty::OuterParty:
		DetectionRadius = 500.0f;
		break;
	case ENPCLoyalty::Prole:
		DetectionRadius = 250.0f;
		break;
	case ENPCLoyalty::ThoughtPolice:
		DetectionRadius = 1000.0f;
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("NPCBase '%s': Initialized. Loyalty=%s DetectionRadius=%.0f"),
		*GetName(),
		*UEnum::GetValueAsString(Loyalty),
		DetectionRadius);
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
	if (ReportCooldownRemaining > 0.0f || !SurveillanceSystem)
	{
		return;
	}

	const float Weight = GetReportWeight();
	SurveillanceSystem->ReportIncident(ESuspicionEvent::CitizenReport, Weight);

	bIsSuspicious             = true;
	ReportCooldownRemaining   = 30.0f; // 30-second report cooldown per NPC

	UE_LOG(LogTemp, Warning,
		TEXT("NPCBase '%s' (%s): Reported player — CitizenReport weight=%.2f"),
		*GetName(), *UEnum::GetValueAsString(Loyalty), Weight);
}

bool ANPCBase::CanSeePlayer() const
{
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

	const FVector MyLoc     = GetActorLocation();
	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	const float   Distance  = FVector::Dist(MyLoc, PlayerLoc);

	if (Distance > DetectionRadius)
	{
		return false;
	}

	// Line-of-sight check — obstructions (walls, furniture) block NPC sight
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(const_cast<ANPCBase*>(this));
	Params.AddIgnoredActor(PlayerPawn);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		MyLoc + FVector(0.f, 0.f, 60.f),
		PlayerLoc + FVector(0.f, 0.f, 60.f),
		ECC_Visibility,
		Params);

	return !bBlocked;
}

void ANPCBase::ExecutePatrol()
{
	if (PatrolRoute.Num() == 0)
	{
		return;
	}

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC)
	{
		return;
	}

	if (bWaypointMoveActive)
	{
		const FVector Target   = PatrolRoute[CurrentWaypointIndex];
		const float   DistLeft = FVector::Dist(GetActorLocation(), Target);

		if (DistLeft <= WaypointAcceptRadius)
		{
			bWaypointMoveActive = false;
			AIC->StopMovement();

			// Pause 2 seconds at each waypoint before moving on
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

	// Check WinstonCharacter state for suspicious actions
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()
		? GetWorld()->GetFirstPlayerController()->GetPawn()
		: nullptr;

	AWinstonCharacter* Winston = Cast<AWinstonCharacter>(PlayerPawn);
	if (!Winston)
	{
		return;
	}

	bool bPlayerSuspicious = false;

	// Writing the diary is an immediately observable suspicious act
	if (Winston->bIsWritingDiary)
	{
		bPlayerSuspicious = true;
	}

	// Being in a restricted area is suspicious
	// (zone actors tag the pawn directly; check the tag)
	if (PlayerPawn->ActorHasTag(FName("InRestrictedZone")))
	{
		bPlayerSuspicious = true;
	}

	if (bPlayerSuspicious)
	{
		// Each loyalty tier has a different threshold before reporting
		const float Threshold = GetReportThreshold();
		if (PlayerObservationTime >= Threshold)
		{
			ReportPlayer();
			PlayerObservationTime = 0.0f; // Reset after reporting
		}
	}
	else
	{
		// Innocent behavior — bleed observation time down
		PlayerObservationTime = FMath::Max(0.0f, PlayerObservationTime - DeltaTime * 0.5f);
	}
}

float ANPCBase::GetReportThreshold() const
{
	// How many seconds of suspicious observation before reporting
	switch (Loyalty)
	{
	case ENPCLoyalty::PartyFaithful:  return 1.0f;  // Hyper-vigilant
	case ENPCLoyalty::OuterParty:     return 3.0f;  // Moderately watchful
	case ENPCLoyalty::Prole:          return 10.0f; // Mostly indifferent
	case ENPCLoyalty::ThoughtPolice:  return 0.5f;  // Immediate detection
	}
	return 5.0f;
}

float ANPCBase::GetReportWeight() const
{
	switch (Loyalty)
	{
	case ENPCLoyalty::PartyFaithful:  return 0.20f;
	case ENPCLoyalty::OuterParty:     return 0.10f;
	case ENPCLoyalty::Prole:          return 0.05f;
	case ENPCLoyalty::ThoughtPolice:  return 0.30f;
	}
	return 0.10f;
}
