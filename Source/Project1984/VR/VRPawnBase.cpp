#include "VRPawnBase.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/Systems/SurveillanceSystem.h"

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
	LocomotionMode   = ELocomotionMode::Teleport;
	SnapTurnAngle    = 45.0f;
	SmoothTurnSpeed  = 90.0f;
	bEnableVignette  = true;
	bSeatedMode      = false;
	bTeleportArcActive = false;
	PendingTeleportLocation = FVector::ZeroVector;

	// Input asset pointers — assigned in Blueprint / DataAsset
	DefaultMappingContext = nullptr;
	IA_Move            = nullptr;
	IA_Turn            = nullptr;
	IA_TeleportAim     = nullptr;
	IA_TeleportConfirm = nullptr;
	IA_GrabLeft        = nullptr;
	IA_GrabRight       = nullptr;
}

void AVRPawnBase::BeginPlay()
{
	Super::BeginPlay();

	// Set VR tracking origin to floor level (standing) or eye level (seated)
	const EHMDTrackingOrigin::Type Origin =
		bSeatedMode ? EHMDTrackingOrigin::Eye : EHMDTrackingOrigin::Floor;
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(Origin);

	// Load comfort settings from game user settings if previously saved
	// (full persistence wired once UGameUserSettings subclass is defined)
	UE_LOG(LogTemp, Log, TEXT("VRPawnBase: Tracking origin set to %s"),
		bSeatedMode ? TEXT("Eye") : TEXT("Floor"));

	// Register Enhanced Input mapping context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AVRPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update teleport arc visualization while the player is aiming
	if (LocomotionMode == ELocomotionMode::Teleport && bTeleportArcActive)
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
	if (!EIC)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPawnBase: EnhancedInputComponent not found — check project input settings."));
		return;
	}

	// Thumbstick movement (smooth locomotion)
	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVRPawnBase::OnMoveInput);
	}

	// Snap / smooth turn
	if (IA_Turn)
	{
		EIC->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &AVRPawnBase::OnTurnInput);
	}

	// Teleport aim: started → show arc, completed → hide arc
	if (IA_TeleportAim)
	{
		EIC->BindAction(IA_TeleportAim, ETriggerEvent::Started,    this,
			[this](const FInputActionValue&) { bTeleportArcActive = true; });
		EIC->BindAction(IA_TeleportAim, ETriggerEvent::Completed,  this,
			[this](const FInputActionValue&) { bTeleportArcActive = false; });
	}

	// Teleport confirm (A / X button)
	if (IA_TeleportConfirm)
	{
		EIC->BindAction(IA_TeleportConfirm, ETriggerEvent::Started, this,
			&AVRPawnBase::OnTeleportConfirmInput);
	}
}

void AVRPawnBase::OnMoveInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	HandleMovementInput(Axis.Y, Axis.X); // Y = forward, X = strafe
}

void AVRPawnBase::OnTurnInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	HandleTurnInput(Axis.X);
}

void AVRPawnBase::OnTeleportConfirmInput(const FInputActionValue& /*Value*/)
{
	if (bTeleportArcActive && !PendingTeleportLocation.IsZero())
	{
		TeleportToLocation(PendingTeleportLocation);
		bTeleportArcActive = false;
	}
}

void AVRPawnBase::TeleportToLocation(FVector TargetLocation)
{
	// Validate against nav mesh before committing
	FNavLocation NavPoint;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys && !NavSys->ProjectPointToNavigation(TargetLocation, NavPoint, FVector(50.f, 50.f, 100.f)))
	{
		UE_LOG(LogTemp, Warning, TEXT("VRPawnBase: Teleport target not on nav mesh — aborted."));
		return;
	}

	// Report to SurveillanceSystem if this is a restricted zone
	// (zone tagging is done via overlap volumes with a IsRestrictedZone flag)
	if (GetGameInstance())
	{
		if (USurveillanceSystem* SS = GetGameInstance()->GetSubsystem<USurveillanceSystem>())
		{
			// Restricted-zone check will be implemented via overlap with ARestrictedZoneVolume.
			// For now the call site is established; zone actors will call ReportIncident directly.
		}
	}

	// Fade out → move → fade in for comfort
	FadeCamera(0.0f, 1.0f, 0.15f);
	SetActorLocation(NavPoint.Location);
	FadeCamera(1.0f, 0.0f, 0.15f);
}

void AVRPawnBase::SetLocomotionMode(ELocomotionMode NewMode)
{
	LocomotionMode = NewMode;

	// Update input mapping context priority so teleport-arc actions
	// are only active in Teleport mode (prevents conflicting bindings)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				// Re-add at priority 0; higher-priority contexts override per-action
				Subsystem->RemoveMappingContext(DefaultMappingContext);
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	bTeleportArcActive = false;
}

void AVRPawnBase::HandleMovementInput(float ForwardValue, float RightValue)
{
	if (LocomotionMode != ELocomotionMode::Smooth)
	{
		return;
	}

	// Move relative to HMD forward direction (camera, not pawn root)
	const FVector Forward = VRCamera->GetForwardVector() * ForwardValue;
	const FVector Right   = VRCamera->GetRightVector()   * RightValue;
	const FVector MoveDir = (Forward + Right).GetClampedToMaxSize(1.0f);

	AddMovementInput(MoveDir, 1.0f);

	// Trigger comfort vignette when moving
	const bool bMoving = MoveDir.SizeSquared() > KINDA_SMALL_NUMBER;
	if (bEnableVignette)
	{
		UpdateComfortVignette(bMoving);
	}
}

void AVRPawnBase::HandleTurnInput(float TurnValue)
{
	if (FMath::Abs(TurnValue) <= 0.5f)
	{
		return;
	}

	if (SnapTurnAngle > 0.0f)
	{
		// Snap turn: brief fade, rotate, fade back
		const float Direction = FMath::Sign(TurnValue);
		FadeCamera(0.0f, 1.0f, 0.08f);
		AddActorWorldRotation(FRotator(0.0f, SnapTurnAngle * Direction, 0.0f));
		FadeCamera(1.0f, 0.0f, 0.08f);
	}
	else
	{
		// Smooth turn using cached DeltaTime is not directly available here.
		// We use a fixed per-frame increment; callers who need DeltaTime-scaled
		// smooth turn should call HandleTurnInput from Tick with a scaled value.
		AddActorWorldRotation(FRotator(0.0f, SmoothTurnSpeed * TurnValue * GetWorld()->GetDeltaSeconds(), 0.0f));

		if (bEnableVignette)
		{
			UpdateComfortVignette(true);
		}
	}
}

void AVRPawnBase::UpdateComfortVignette(bool bIsMoving)
{
	// The vignette is driven by a post-process material applied to the VR camera.
	// The material exposes a scalar parameter "VignetteStrength" (0=off, 1=full).
	// We set it here; the actual UMaterialInstanceDynamic is created in Blueprint
	// and stored as a component on this pawn.

	// Blueprint-callable alternative:
	// Cast<UPostProcessComponent>(GetComponentByClass(UPostProcessComponent::StaticClass()))
	//   ->Settings.VignetteIntensity = bIsMoving ? 0.8f : 0.0f;

	// For now log the state change; full implementation is Blueprint-side
	// once the post-process component is wired up in the pawn Blueprint.
	UE_LOG(LogTemp, Verbose, TEXT("VRPawnBase: ComfortVignette %s"),
		bIsMoving ? TEXT("ON") : TEXT("OFF"));
}

void AVRPawnBase::FadeCamera(float FromAlpha, float ToAlpha, float Duration)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(
				FromAlpha, ToAlpha, Duration, FLinearColor::Black,
				/*bHoldWhenFinished=*/false, /*bForceCompletion=*/false);
		}
	}
}
