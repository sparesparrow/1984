#include "VRPawnBase.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"

AVRPawnBase::AVRPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// VR Camera
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(RootComponent);

	// Motion Controllers
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(RootComponent);
	LeftController->SetTrackingMotionSource(FName("Left"));

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(RootComponent);
	RightController->SetTrackingMotionSource(FName("Right"));

	// Comfort defaults
	LocomotionMode = ELocomotionMode::Teleport;
	SnapTurnAngle = 45.0f;
	bEnableVignette = true;
	bSeatedMode = false;
}

void AVRPawnBase::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Initialize VR tracking origin (floor or eye level based on seated mode)
	// TODO: Load comfort settings from save data
	// TODO: Spawn hand meshes or hand tracking visualization
}

void AVRPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: Update hand tracking data if available
	// TODO: Update teleport arc visualization if in teleport mode
}

void AVRPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// TODO: Bind Enhanced Input actions for:
	// - Thumbstick: movement/teleport aim
	// - Trigger: interact/select
	// - Grip: grab objects
	// - A/X buttons: teleport confirm
	// - B/Y buttons: menu/comfort settings
	// - Thumbstick press: snap turn
}

void AVRPawnBase::TeleportToLocation(FVector TargetLocation)
{
	// TODO: Validate target location (navigation mesh check)
	// TODO: Play teleport fade effect
	// TODO: Move pawn to target
	// TODO: Report movement to SurveillanceSystem (entering restricted areas)
	SetActorLocation(TargetLocation);
}

void AVRPawnBase::SetLocomotionMode(ELocomotionMode NewMode)
{
	LocomotionMode = NewMode;

	// TODO: Update input bindings for the new mode
	// TODO: Show/hide teleport arc component
}

void AVRPawnBase::HandleMovementInput(float ForwardValue, float RightValue)
{
	if (LocomotionMode == ELocomotionMode::Smooth)
	{
		// TODO: Apply smooth movement relative to HMD forward direction
		// TODO: Clamp speed for Quest performance
		// TODO: Trigger comfort vignette if enabled
	}
}

void AVRPawnBase::HandleTurnInput(float TurnValue)
{
	if (FMath::Abs(TurnValue) > 0.5f)
	{
		if (SnapTurnAngle > 0.0f)
		{
			// TODO: Apply snap turn by SnapTurnAngle degrees
			// TODO: Brief screen fade for comfort
		}
		else
		{
			// TODO: Apply smooth turn
			// TODO: Trigger comfort vignette if enabled
		}
	}
}

void AVRPawnBase::UpdateComfortVignette(bool bIsMoving)
{
	// TODO: Show/hide post-process vignette material based on movement
	// TODO: Fade in/out smoothly to reduce VR sickness
}
