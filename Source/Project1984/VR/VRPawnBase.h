#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "VRPawnBase.generated.h"

class UCameraComponent;
class UMotionControllerComponent;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class ELocomotionMode : uint8
{
	Teleport	UMETA(DisplayName = "Teleport"),
	Smooth		UMETA(DisplayName = "Smooth"),
	RoomScale	UMETA(DisplayName = "Room Scale")
};

UCLASS()
class PROJECT1984_API AVRPawnBase : public APawn
{
	GENERATED_BODY()

public:
	AVRPawnBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// -----------------------------------------------------------------------
	// Components
	// -----------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UCameraComponent* VRCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UMotionControllerComponent* LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UMotionControllerComponent* RightController;

	// -----------------------------------------------------------------------
	// Comfort settings
	// -----------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	ELocomotionMode LocomotionMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	float SnapTurnAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	float SmoothTurnSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	bool bEnableVignette;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Comfort")
	bool bSeatedMode;

	// -----------------------------------------------------------------------
	// Enhanced Input — assign data assets in BP child class or class defaults
	// -----------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_TeleportActivate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_SnapTurnLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_SnapTurnRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_GrabLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_GrabRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_Interact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Input")
	UInputAction* IA_PauseMenu;

	// -----------------------------------------------------------------------
	// Locomotion
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "VR|Locomotion")
	void TeleportToLocation(FVector TargetLocation);

	UFUNCTION(BlueprintCallable, Category = "VR|Locomotion")
	void SetLocomotionMode(ELocomotionMode NewMode);

protected:
	FVector PendingTeleportLocation;
	bool    bTeleportPending;

	void OnMoveInput(const FInputActionValue& Value);
	void OnTeleportActivate(const FInputActionValue& Value);
	void OnSnapTurnLeft(const FInputActionValue& Value);
	void OnSnapTurnRight(const FInputActionValue& Value);
	void OnGrabLeft(const FInputActionValue& Value);
	void OnGrabRight(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnPauseMenu(const FInputActionValue& Value);

	void HandleMovementInput(float ForwardValue, float RightValue);
	void HandleTurnInput(float TurnValue);
	void UpdateComfortVignette(bool bIsMoving);
};
