#include "NPCBase.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Loyalty = ENPCLoyalty::OuterParty;
	DetectionRadius = 500.0f;
	bIsSuspicious = false;
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Initialize AI controller with patrol route from level data
	// TODO: Set detection parameters based on loyalty type
}

void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanSeePlayer())
	{
		EvaluatePlayerBehavior();
	}

	ExecutePatrol();
}

void ANPCBase::ReportPlayer()
{
	// TODO: Send suspicion event to SurveillanceSystem
	// Weight varies by NPC loyalty:
	// PartyFaithful: +0.20, OuterParty: +0.10, ThoughtPolice: +0.30
	bIsSuspicious = true;
}

bool ANPCBase::CanSeePlayer() const
{
	// TODO: Line-of-sight check within DetectionRadius
	// TODO: Account for obstructions (walls, furniture, crowd)
	return false;
}

void ANPCBase::ExecutePatrol()
{
	// TODO: Follow assigned patrol route using AI navigation
	// TODO: Pause at observation points
	// TODO: React to nearby suspicion events
}

void ANPCBase::EvaluatePlayerBehavior()
{
	// TODO: Check if player is performing suspicious actions
	// (writing diary, speaking Oldspeak, in restricted area, etc.)
	// TODO: Threshold based on loyalty — Proles ignore most behavior
}
