#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VisionConeUtils.generated.h"

/**
 * UVisionConeUtils
 *
 * Static utility library for vision cone and line-of-sight checks.
 * Shared by TelescreenComponent, ThoughtPoliceAI, and NPCBase to avoid
 * duplicating the same geometry + trace logic across every surveillance actor.
 *
 * All functions are Blueprint-callable for prototyping in the editor.
 */
UCLASS()
class PROJECT1984_API UVisionConeUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns true if Target is within Observer's vision cone.
	 *
	 * @param Observer          The watching actor (telescreen, NPC, Thought Police pawn).
	 * @param Target            The watched actor (Winston's pawn).
	 * @param ConeHalfAngleDeg  Half-angle of the cone in degrees (e.g. 45 = 90° total FOV).
	 * @param MaxRange          Maximum detection distance in cm.
	 * @param bRequireLineOfSight  If true, performs a blocking trace; obstructions block detection.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Utils")
	static bool IsActorInVisionCone(
		const AActor* Observer,
		const AActor* Target,
		float ConeHalfAngleDeg,
		float MaxRange,
		bool bRequireLineOfSight = true);

	/**
	 * Returns true if a world-space location is inside Observer's vision cone.
	 * Useful for checking whether a point of interest (e.g. door, restricted zone
	 * boundary) is within a surveillance camera's field of view.
	 * Does NOT perform a line-of-sight trace (no target actor to ignore).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "1984|Utils")
	static bool IsLocationInVisionCone(
		const AActor* Observer,
		FVector TargetLocation,
		float ConeHalfAngleDeg,
		float MaxRange);
};
