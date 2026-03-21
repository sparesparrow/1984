#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VRPawnBase.generated.h"

class UCameraComponent;
class UMotionControllerComponent;
class USphereComponent;

/**
 * ELocomotionMode
 *
 * VR locomotion options for comfort settings.
 */
UENUM(BlueprintType)
enum class ELocomotionMode : uint8
{
	/** Teleport locomotion (default, most comfortable) */
	Teleport	UMETA(DisplayName = "Teleport"),
	/** Smooth continuous movement */
	Smooth		UMETA(DisplayName = "Smooth"),
	/** Room-scale only (physical movement) */
	RoomScale	UMETA(DisplayName = "Room Scale")
};

/**
 * AVRPawnBase
 *
 * Mobile-optimised VR pawn targeting Meta Quest 2/3/Pro.
 * Supports teleport and smooth locomotion, hand tracking,
 * controller input, and comfort settings (vignette, snap turn,
 * seated/standing mode).
 */
UCLASS()
class PROJECT1984_API AVRPawnBase : public APawn
{
	GENERATED_BODY()

public:
	AVRPawnBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** VR Camera (head-mounted display) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UCameraComponent* VRCamera;

	/** Left motion controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UMotionControllerComponent* LeftController;

	/** Right motion controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UMotionControllerComponent* RightController;

	/** Current locomotion mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	ELocomotionMode LocomotionMode;

	/** Snap turn angle in degrees (0 = smooth turn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	float SnapTurnAngle;

	/** Whether to show vignette during movement (comfort) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	bool bEnableVignette;

	/** Whether in seated mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	bool bSeatedMode;

	/** Execute teleport to target location */
	UFUNCTION(BlueprintCallable, Category = "VR|Locomotion")
	void TeleportToLocation(FVector TargetLocation);

	/** Toggle between locomotion modes */
	UFUNCTION(BlueprintCallable, Category = "VR|Locomotion")
	void SetLocomotionMode(ELocomotionMode NewMode);

protected:
	/** Handle thumbstick movement input */
	void HandleMovementInput(float ForwardValue, float RightValue);

	/** Handle snap/smooth turn input */
	void HandleTurnInput(float TurnValue);

	/** Update comfort vignette based on movement */
	void UpdateComfortVignette(bool bIsMoving);
};
