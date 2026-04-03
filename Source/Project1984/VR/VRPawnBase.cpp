#include "VRPawnBase.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

AVRPawnBase::AVRPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- VR Camera ---
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(RootComponent);

	// --- Motion Controllers ---
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(RootComponent);
	LeftController->SetTrackingMotionSource(FName("Left"));

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(RootComponent);
	RightController->SetTrackingMotionSource(FName("Right"));

	// --- Comfort defaults ---
	LocomotionMode  = ELocomotionMode::Teleport;
	SnapTurnAngle   = 45.0f;
	bEnableVignette = true;
	bSeatedMode     = false;

	// --- Enhanced Input (data assets assigned in BP child class) ---
	DefaultMappingContext = nullptr;
	IA_Move               = nullptr;
	IA_TeleportActivate   = nullptr;
	IA_SnapTurnLeft       = nullptr;
	IA_SnapTurnRight      = nullptr;
	IA_GrabLeft           = nullptr;
	IA_GrabRight          = nullptr;
	IA_Interact           = nullptr;
	IA_PauseMenu          = nullptr;

	bTeleportPending         = false;
	PendingTeleportLocation  = FVector::ZeroVector;
}

void AVRPawnBase::BeginPlay()
{
	Super::BeginPlay();

	// Set tracking origin: Stage (eye-level) for seated, Floor for standing.
	const EHMDTrackingOrigin::Type Origin =
		bSeatedMode ? EHMDTrackingOrigin::Stage : EHMDTrackingOrigin::Floor;
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(Origin);

	// Add the default mapping context so Quest controller bindings are active.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, /*Priority=*/0);
			}
		}
	}
}

void AVRPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update teleport arc visualization while the player is aiming
	if (LocomotionMode == ELocomotionMode::Teleport && bTeleportPending)
	{
		// Project a nav-mesh point along the right controller's forward vector.
		// Full spline-arc visualization is Blueprint-side; here we update the
		// candidate destination so Blueprint can draw the arc.
		const FVector AimOrigin    = RightController->GetComponentLocation();
		const FVector AimDirection = RightController->GetForwardVector();
		const FVector Candidate    = AimOrigin + AimDirection * 500.0f;

		FNavLocation NavPoint;
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys && NavSys->ProjectPointToNavigation(Candidate, NavPoint, FVector(50.f, 50.f, 100.f)))
		{
			PendingTeleportLocation = NavPoint.Location;
		}
	}
}

void AVRPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return; // Fallback: non-Enhanced Input not supported

	// Bind each action only if the data asset is assigned.
	if (IA_Move)
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVRPawnBase::OnMoveInput);

	if (IA_TeleportActivate)
		EIC->BindAction(IA_TeleportActivate, ETriggerEvent::Completed, this, &AVRPawnBase::OnTeleportActivate);

	if (IA_SnapTurnLeft)
		EIC->BindAction(IA_SnapTurnLeft, ETriggerEvent::Triggered, this, &AVRPawnBase::OnSnapTurnLeft);

	if (IA_SnapTurnRight)
		EIC->BindAction(IA_SnapTurnRight, ETriggerEvent::Triggered, this, &AVRPawnBase::OnSnapTurnRight);

	if (IA_GrabLeft)
		EIC->BindAction(IA_GrabLeft, ETriggerEvent::Started, this, &AVRPawnBase::OnGrabLeft);

	if (IA_GrabRight)
		EIC->BindAction(IA_GrabRight, ETriggerEvent::Started, this, &AVRPawnBase::OnGrabRight);

	if (IA_Interact)
		EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &AVRPawnBase::OnInteract);

	if (IA_PauseMenu)
		EIC->BindAction(IA_PauseMenu, ETriggerEvent::Started, this, &AVRPawnBase::OnPauseMenu);
}

// ---------------------------------------------------------------------------
// Enhanced Input callbacks
// ---------------------------------------------------------------------------

void AVRPawnBase::OnMoveInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	HandleMovementInput(Axis.Y, Axis.X);
}

void AVRPawnBase::OnTeleportActivate(const FInputActionValue& /*Value*/)
{
	if (bTeleportPending && LocomotionMode == ELocomotionMode::Teleport)
	{
		TeleportToLocation(PendingTeleportLocation);
		bTeleportPending = false;
	}
}

void AVRPawnBase::OnSnapTurnLeft(const FInputActionValue& /*Value*/)
{
	HandleTurnInput(-1.0f);
}

void AVRPawnBase::OnSnapTurnRight(const FInputActionValue& /*Value*/)
{
	HandleTurnInput(1.0f);
}

void AVRPawnBase::OnGrabLeft(const FInputActionValue& /*Value*/)
{
	// Delegate to HandInteraction component on LeftController.
	// The component is added in the Blueprint child class.
}

void AVRPawnBase::OnGrabRight(const FInputActionValue& /*Value*/)
{
	// Delegate to HandInteraction component on RightController.
}

void AVRPawnBase::OnInteract(const FInputActionValue& /*Value*/)
{
	// World interaction (read sign, open door, pick up diary) — handled in Blueprint.
}

void AVRPawnBase::OnPauseMenu(const FInputActionValue& /*Value*/)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetPause(!PC->IsPaused());
	}
}

// ---------------------------------------------------------------------------
// Locomotion
// ---------------------------------------------------------------------------

void AVRPawnBase::TeleportToLocation(FVector TargetLocation)
{
	// Project onto nav mesh — reject destinations outside navigable area.
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		const bool bValid = NavSys->ProjectPointToNavigation(
			TargetLocation, NavLoc, FVector(50.f, 50.f, 200.f));
		if (!bValid) return;
		TargetLocation = NavLoc.Location;
	}

	// Brief vignette flash (hides the position pop for comfort).
	UpdateComfortVignette(true);
	SetActorLocation(TargetLocation, /*bSweep=*/false);
	UpdateComfortVignette(false);
}

void AVRPawnBase::SetLocomotionMode(ELocomotionMode NewMode)
{
	LocomotionMode = NewMode;
}

void AVRPawnBase::HandleMovementInput(float ForwardValue, float RightValue)
{
	if (LocomotionMode != ELocomotionMode::Smooth) return;

	// Move relative to the HMD look direction, flattened to the horizontal plane
	// so the player cannot "fly" by looking up or down.
	const FVector HMDForward = FVector(VRCamera->GetForwardVector().X, VRCamera->GetForwardVector().Y, 0.f).GetSafeNormal();
	const FVector HMDRight   = FVector(VRCamera->GetRightVector().X,   VRCamera->GetRightVector().Y,   0.f).GetSafeNormal();

	// Conservative speed for Quest comfort: ~2 m/s walking pace.
	const float MoveSpeed = 200.0f;
	const float Dt        = GetWorld()->GetDeltaSeconds();

	AddMovementInput(HMDForward, ForwardValue * MoveSpeed * Dt);
	AddMovementInput(HMDRight,   RightValue   * MoveSpeed * Dt);

	const bool bMoving = FMath::Abs(ForwardValue) > 0.1f || FMath::Abs(RightValue) > 0.1f;
	if (bEnableVignette)
	{
		UpdateComfortVignette(bMoving);
	}
}

void AVRPawnBase::HandleTurnInput(float TurnValue)
{
	if (FMath::Abs(TurnValue) <= 0.5f) return;

	if (SnapTurnAngle > 0.0f)
	{
		// Snap turn: instant rotation by SnapTurnAngle degrees.
		const float Sign = TurnValue > 0.f ? 1.f : -1.f;
		AddActorWorldRotation(FRotator(0.f, SnapTurnAngle * Sign, 0.f));
		// Brief vignette for comfort.
		if (bEnableVignette) UpdateComfortVignette(true);
	}
	else
	{
		// Smooth turn.
		const float SmoothRate = 90.0f; // degrees/s
		AddActorWorldRotation(FRotator(0.f, TurnValue * SmoothRate * GetWorld()->GetDeltaSeconds(), 0.f));
		if (bEnableVignette) UpdateComfortVignette(FMath::Abs(TurnValue) > 0.1f);
	}
}

void AVRPawnBase::UpdateComfortVignette(bool bIsMoving)
{
	if (!VRCamera) return;

	// Control the built-in post-process vignette on the VR camera.
	// A value of 0.4 is the neutral/always-on subtle vignette;
	// 1.5 is the strong comfort vignette shown during locomotion.
	FPostProcessSettings& PP = VRCamera->PostProcessSettings;
	PP.bOverride_VignetteIntensity = true;
	PP.VignetteIntensity = (bIsMoving && bEnableVignette) ? 1.5f : 0.4f;
}
