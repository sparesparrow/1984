#include "TelescreenComponent.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"

UTelescreenComponent::UTelescreenComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	State = ETelescreenState::Broadcasting;
	VisionConeAngle = 60.0f;
	SurveillanceRange = 800.0f;
	bPlayerInView = false;
}

void UTelescreenComponent::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Register with SurveillanceSystem as a surveillance source
	// TODO: Start propaganda video playback loop
	// TODO: Initialize directional audio emitter
}

void UTelescreenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State != ETelescreenState::Off && State != ETelescreenState::Obscured)
	{
		UpdateSurveillance();
		UpdateBroadcast();
	}
}

void UTelescreenComponent::StartTwoMinutesHate()
{
	State = ETelescreenState::TwoMinutesHate;

	// TODO: Switch video content to Goldstein's face / hate montage
	// TODO: Increase audio volume
	// TODO: Monitor player participation (fist gesture, shouting)
	// TODO: Report AttendingTwoMinutesHate event if player participates (-0.10 suspicion)
}

void UTelescreenComponent::EndTwoMinutesHate()
{
	State = ETelescreenState::Broadcasting;

	// TODO: Return to normal propaganda loop
	// TODO: Check if player participated — non-participation is suspicious
}

void UTelescreenComponent::ObscureTelescreen()
{
	if (State == ETelescreenState::Broadcasting || State == ETelescreenState::Addressing)
	{
		State = ETelescreenState::Obscured;

		// TODO: Report high suspicion event — obscuring a telescreen is very risky
		// TODO: Visual feedback — screen goes dark/muffled
		// TODO: Start timer — telescreen auto-restores after a period
		// TODO: Thought Police may be dispatched
	}
}

bool UTelescreenComponent::IsPlayerVisible() const
{
	// TODO: Get player pawn location
	// TODO: Calculate angle between telescreen forward and direction to player
	// TODO: Check within VisionConeAngle and SurveillanceRange
	// TODO: Line-of-sight trace for obstructions
	return bPlayerInView;
}

void UTelescreenComponent::UpdateSurveillance()
{
	bool bWasVisible = bPlayerInView;
	bPlayerInView = IsPlayerVisible();

	if (bPlayerInView && !bWasVisible)
	{
		// Player just entered view — report observation event
		// TODO: Report TelescreenObservation to SurveillanceSystem (+0.08)
	}

	if (bPlayerInView)
	{
		// TODO: Occasionally switch to Addressing state (direct the player)
		// TODO: Quest Pro: check eye tracking — is player looking at telescreen?
	}
}

void UTelescreenComponent::UpdateBroadcast()
{
	// TODO: Cycle through propaganda content:
	// - Party slogans: "WAR IS PEACE", "FREEDOM IS SLAVERY", "IGNORANCE IS STRENGTH"
	// - Big Brother imagery
	// - Production statistics (fabricated)
	// - Enemy updates (Eurasia/Eastasia)
	// TODO: Spatialize audio from telescreen location
}
