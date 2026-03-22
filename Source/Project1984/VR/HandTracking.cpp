#include "HandTracking.h"

UHandTracking::UHandTracking()
{
	PrimaryComponentTick.bCanEverTick = true;

	bHandTrackingActive = false;
	CurrentGesture = EHandGesture::None;
	PreviousGesture = EHandGesture::None;
	PinchStrength = 0.0f;
	GrabStrength = 0.0f;
	bIsLeftHand = true;
}

void UHandTracking::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Check if OpenXR hand tracking extension is available
	// TODO: Initialize hand tracking data structures
	// TODO: If not available, fall back to controller input
}

void UHandTracking::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTrackingData();

	EHandGesture NewGesture = ClassifyGesture();
	if (NewGesture != CurrentGesture)
	{
		PreviousGesture = CurrentGesture;
		CurrentGesture = NewGesture;
		OnGestureChanged.Broadcast(CurrentGesture, PreviousGesture);
	}
}

bool UHandTracking::IsHandTrackingSupported() const
{
	// TODO: Query OpenXR runtime for hand tracking extension support
	// TODO: Check Meta Quest device capabilities
	return false;
}

EHandGesture UHandTracking::GetCurrentGesture() const
{
	return CurrentGesture;
}

void UHandTracking::UpdateTrackingData()
{
	// TODO: Read skeletal hand tracking data from OpenXR
	// TODO: Extract joint positions and rotations for all 26 hand joints
	// TODO: Calculate pinch strength (thumb-index distance)
	// TODO: Calculate grab strength (average finger curl)
	// TODO: Update bHandTrackingActive based on tracking confidence
}

EHandGesture UHandTracking::ClassifyGesture() const
{
	// TODO: Classify gesture based on joint positions:
	// Pinch: thumb tip near index tip, other fingers relaxed
	// Grab: all fingers curled > threshold
	// Point: index extended, others curled
	// Open: all fingers extended
	// ThumbsUp: thumb extended, others curled (Party salute)
	// Fist: all fingers tightly curled (Two Minutes Hate)

	if (PinchStrength > 0.8f)
	{
		return EHandGesture::Pinch;
	}
	if (GrabStrength > 0.8f)
	{
		return EHandGesture::Grab;
	}

	return EHandGesture::None;
}
