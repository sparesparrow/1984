#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandInteraction.generated.h"

class AWinstonCharacter;

/**
 * EGrabState
 *
 * States for hand-based object interaction.
 */
UENUM(BlueprintType)
enum class EGrabState : uint8
{
	/** Hand is empty, ready to grab */
	Empty		UMETA(DisplayName = "Empty"),
	/** Hand is near a grabbable object */
	Hovering	UMETA(DisplayName = "Hovering"),
	/** Hand is holding an object */
	Grabbing	UMETA(DisplayName = "Grabbing"),
	/** Hand is examining a held object (brought close to face) */
	Examining	UMETA(DisplayName = "Examining")
};

/**
 * UHandInteraction
 *
 * Component for VR hand-based interactions: pickup, examine, read,
 * and write (physical diary interaction). Supports both controller
 * grip input and hand tracking pinch/grab gestures.
 *
 * The component lives on the VR pawn and should be attached to the
 * MotionControllerComponent so its location tracks the hand.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT1984_API UHandInteraction : public UActorComponent
{
	GENERATED_BODY()

public:
	UHandInteraction();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Current grab state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Interaction")
	EGrabState GrabState;

	/** Currently held actor (if any) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Interaction")
	AActor* HeldActor;

	/** Grab radius in cm (default 15 cm — tight for precise VR interaction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction")
	float GrabRadius;

	/** Distance from camera at which Grabbing transitions to Examining */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction")
	float ExamineDistance;

	/** Socket name on the hand mesh to attach held objects to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction")
	FName GrabSocketName;

	/** Attempt to grab the nearest interactable object */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction")
	void TryGrab();

	/** Release the currently held object */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction")
	void Release();

	/** Check if holding an object */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VR|Interaction")
	bool IsHolding() const;

	/** Notify that the player participated in Two Minutes Hate (fist gesture from hand tracking) */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction")
	void NotifyHateParticipation();

protected:
	/** Nearest hoverable actor this frame */
	AActor* HoverActor;

	/** Ring buffer of recent hand velocities for throw calculation (world units/s) */
	TArray<FVector> VelocityHistory;
	static constexpr int32 VelocityHistorySize = 6;

	/** Previous hand world location for velocity tracking */
	FVector PrevHandLocation;

	/** Camera component cache (resolved in BeginPlay) for Examining distance check */
	class UCameraComponent* VRCamera;

	/** Find the nearest grabbable actor within GrabRadius */
	AActor* FindNearestGrabbable() const;

	/** Attach held object to hand */
	void AttachToHand(AActor* Target);

	/** Detach held object from hand */
	void DetachFromHand();

	/** Compute average throw velocity from VelocityHistory */
	FVector ComputeThrowVelocity() const;
};
