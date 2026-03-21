#include "Project1984GameMode.h"
#include "Project1984/VR/VRPawnBase.h"

AProject1984GameMode::AProject1984GameMode()
{
	DefaultPawnClass = AVRPawnBase::StaticClass();
	CurrentAct = 1;
	bPlayerCaptured = false;
}

void AProject1984GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// TODO: Load saved game state if continuing a session
	InitializeSubsystems();
}

void AProject1984GameMode::StartPlay()
{
	Super::StartPlay();

	// TODO: Trigger Act I opening sequence (telescreen morning broadcast)
	// TODO: Initialize surveillance coverage for the current map
}

int32 AProject1984GameMode::GetCurrentAct() const
{
	return CurrentAct;
}

void AProject1984GameMode::AdvanceAct()
{
	if (CurrentAct < 5)
	{
		CurrentAct++;
		// TODO: Trigger narrative transition cinematic
		// TODO: Update surveillance intensity based on act
		// TODO: Unlock new areas/mechanics per act
	}
}

bool AProject1984GameMode::IsPlayerCaptured() const
{
	return bPlayerCaptured;
}

void AProject1984GameMode::InitializeSubsystems()
{
	// TODO: Ensure SurveillanceSystem subsystem is active
	// TODO: Ensure NarrativeManager subsystem is active
	// TODO: Load act-specific configuration
}
