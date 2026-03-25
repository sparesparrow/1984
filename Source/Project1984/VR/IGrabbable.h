#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGrabbable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UGrabbable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IGrabbable
 *
 * Interface for any world object that can be physically picked up by the
 * player's VR hands. Implement on Blueprints or C++ actors that should be
 * interactable: the diary, pamphlets, tools, Party artefacts.
 */
class PROJECT1984_API IGrabbable
{
	GENERATED_BODY()

public:
	/** Whether this object can currently be grabbed (e.g., not locked, in range) */
	UFUNCTION(BlueprintNativeEvent, Category = "VR|Interaction")
	bool CanBeGrabbed() const;

	/** Called by UHandInteraction when the object is grabbed */
	UFUNCTION(BlueprintNativeEvent, Category = "VR|Interaction")
	void OnGrabbed(AActor* GrabbingHand);

	/** Called by UHandInteraction when the object is released */
	UFUNCTION(BlueprintNativeEvent, Category = "VR|Interaction")
	void OnReleased(AActor* ReleasingHand);

	/** The narrative ID reported to the decision log when grabbed (empty = no log) */
	UFUNCTION(BlueprintNativeEvent, Category = "VR|Interaction")
	FString GetGrabDecisionID() const;
};
