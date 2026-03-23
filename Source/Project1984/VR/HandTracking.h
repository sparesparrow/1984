#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandTracking.generated.h"

/**
 * EHandGesture
 *
 * Recognized hand gestures for Meta Quest hand tracking.
 */
UENUM(BlueprintType)
enum class EHandGesture : uint8
{
	/** No specific gesture detected */
	None		UMETA(DisplayName = "None"),
	/** Pinch (thumb + index) — used for selection */
	Pinch		UMETA(DisplayName = "Pinch"),
	/** Grab (all fingers closed) — used for picking up objects */
	Grab		UMETA(DisplayName = "Grab"),
	/** Point (index extended) — used for UI interaction */
	Point		UMETA(DisplayName = "Point"),
	/** Open hand (all fingers extended) — used for release */
	OpenHand	UMETA(DisplayName = "Open Hand"),
	/** Thumbs up — used for Party salute / approval */
	ThumbsUp	UMETA(DisplayName = "Thumbs Up"),
	/** Fist — used for Two Minutes Hate participation */
	Fist		UMETA(DisplayName = "Fist")
};

/**
 * UHandTracking
 *
 * Component for Meta Quest hand tracking integration via OpenXR.
 * Provides full skeletal hand tracking, gesture recognition, and
 * falls back to controller input when hands are not detected.
 *
 * Joint layout (26 joints per hand, EHandKeypoint order):
 *   0  Palm      1  Wrist
 *   2-5  Thumb (metacarpal → tip)
 *   6-9  Index   10-13  Middle  14-17  Ring  18-21  Little
 *   22-25  Extra joints (orientation helpers)
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT1984_API UHandTracking : public UActorComponent
{
	GENERATED_BODY()

public:
	UHandTracking();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---------------------------------------------------------------
	// State
	// ---------------------------------------------------------------

	/** Whether hand tracking is currently available and active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	bool bHandTrackingActive;

	/** Whether falling back to controller input (hand tracking unavailable) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	bool bUsingControllerFallback;

	/** Current detected gesture for this hand */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	EHandGesture CurrentGesture;

	/** Pinch strength (0.0 to 1.0) — thumb-to-index proximity */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	float PinchStrength;

	/** Grab strength (0.0 to 1.0) — average finger curl */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	float GrabStrength;

	/** Whether this is the left or right hand */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|HandTracking")
	bool bIsLeftHand;

	/** World-space joint positions (26 joints) updated each frame when active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	TArray<FVector> JointPositions;

	/** Joint tracking validity flags (one per joint) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|HandTracking")
	TArray<bool> JointTrackingValid;

	// ---------------------------------------------------------------
	// Public API
	// ---------------------------------------------------------------

	/** Check if hand tracking hardware is available */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VR|HandTracking")
	bool IsHandTrackingSupported() const;

	/** Get the current gesture */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VR|HandTracking")
	EHandGesture GetCurrentGesture() const;

	/** Delegate for gesture changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGestureChanged, EHandGesture, NewGesture, EHandGesture, PreviousGesture);

	UPROPERTY(BlueprintAssignable, Category = "VR|HandTracking")
	FOnGestureChanged OnGestureChanged;

protected:
	/** Previous gesture for change detection */
	EHandGesture PreviousGesture;

	/** Maximum thumb-index distance that counts as a full pinch (cm) */
	static constexpr float MaxPinchDistanceCm = 8.0f;

	/** Update hand tracking data from OpenXR */
	void UpdateTrackingData();

	/** Classify current hand pose into a gesture */
	EHandGesture ClassifyGesture() const;

	/** Estimate finger curl for a single finger given its proximal/middle/tip joints */
	float EstimateFingerCurl(int32 ProximalIdx, int32 MiddleIdx, int32 TipIdx) const;
};
