#include "WinstonCharacter.h"
#include "Project1984/Core/Systems/SuspicionComponent.h"

AWinstonCharacter::AWinstonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SuspicionComponent = CreateDefaultSubobject<USuspicionComponent>(TEXT("SuspicionComponent"));

	bIsWritingDiary = false;
	bHasDiary = false;
}

void AWinstonCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Initialize Winston's starting state based on current act
	// TODO: Bind VR motion controller input to character actions
}

void AWinstonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSurveillanceStatus();

	// TODO: Update facial expression state for eye-tracking suspicion (Quest Pro)
}

void AWinstonCharacter::StartDiaryWriting()
{
	bIsWritingDiary = true;
	// TODO: Report diary writing as suspicion event (+0.15 weight)
	// TODO: Enable diary VR interaction (physical book + pen)
}

void AWinstonCharacter::StopDiaryWriting()
{
	bIsWritingDiary = false;
	// TODO: Save diary entry content for decision log
}

bool AWinstonCharacter::IsBeingWatched() const
{
	// TODO: Query SurveillanceSystem for active surveillance on player location
	return false;
}

void AWinstonCharacter::UpdateSurveillanceStatus()
{
	// TODO: Check line-of-sight from nearby telescreens
	// TODO: Check proximity to Thought Police NPCs
	// TODO: Check if citizen NPCs are observing suspicious behavior
}
