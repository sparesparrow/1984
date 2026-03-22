#include "HandInteraction.h"

UHandInteraction::UHandInteraction()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrabState = EGrabState::Empty;
	HeldActor = nullptr;
	GrabRadius = 15.0f; // Tight radius for precise VR interaction
}

void UHandInteraction::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Initialize overlap sphere for grab detection
}

void UHandInteraction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GrabState == EGrabState::Empty)
	{
		// TODO: Check for nearby grabbable objects, update to Hovering state
		AActor* Nearest = FindNearestGrabbable();
		if (Nearest)
		{
			GrabState = EGrabState::Hovering;
			// TODO: Show highlight on nearest object
		}
	}
	else if (GrabState == EGrabState::Grabbing && HeldActor)
	{
		// TODO: Check if object is close to face → transition to Examining
		// TODO: Update held object physics/position
	}
}

void UHandInteraction::TryGrab()
{
	if (GrabState == EGrabState::Empty || GrabState == EGrabState::Hovering)
	{
		AActor* Target = FindNearestGrabbable();
		if (Target)
		{
			AttachToHand(Target);
			GrabState = EGrabState::Grabbing;

			// TODO: If grabbed object is the diary, notify WinstonCharacter
			// TODO: Play grab haptic feedback
		}
	}
}

void UHandInteraction::Release()
{
	if (HeldActor)
	{
		DetachFromHand();
		GrabState = EGrabState::Empty;

		// TODO: Apply throw velocity if hand was moving
		// TODO: Play release haptic feedback
	}
}

bool UHandInteraction::IsHolding() const
{
	return HeldActor != nullptr;
}

AActor* UHandInteraction::FindNearestGrabbable() const
{
	// TODO: Overlap sphere check for actors implementing IGrabbable interface
	// TODO: Filter by distance and line-of-sight from hand
	// TODO: Prioritize objects the hand is pointing at
	return nullptr;
}

void UHandInteraction::AttachToHand(AActor* Target)
{
	HeldActor = Target;
	// TODO: Attach target to hand socket with physics constraint
	// TODO: Disable target collision with player
}

void UHandInteraction::DetachFromHand()
{
	// TODO: Re-enable target collision
	// TODO: Detach from hand socket
	HeldActor = nullptr;
}
