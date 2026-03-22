#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandInteraction.generated.h"

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

	/** Grab radius for detecting nearby interactable objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Interaction")
	float GrabRadius;

	/** Attempt to grab the nearest interactable object */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction")
	void TryGrab();

	/** Release the currently held object */
	UFUNCTION(BlueprintCallable, Category = "VR|Interaction")
	void Release();

	/** Check if holding an object */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VR|Interaction")
	bool IsHolding() const;

protected:
	/** Find the nearest grabbable actor within GrabRadius */
	AActor* FindNearestGrabbable() const;

	/** Attach held object to hand */
	void AttachToHand(AActor* Target);

	/** Detach held object from hand */
	void DetachFromHand();
};
