#include "VisionConeUtils.h"
#include "Engine/World.h"

bool UVisionConeUtils::IsActorInVisionCone(
	const AActor* Observer,
	const AActor* Target,
	float ConeHalfAngleDeg,
	float MaxRange,
	bool bRequireLineOfSight)
{
	if (!Observer || !Target) return false;

	const FVector ObserverLoc = Observer->GetActorLocation();
	const FVector TargetLoc   = Target->GetActorLocation();
	const FVector ToTarget    = TargetLoc - ObserverLoc;

	// --- Distance check (fast reject) ---
	const float DistSq = ToTarget.SizeSquared();
	if (DistSq > FMath::Square(MaxRange)) return false;

	// --- Angle check ---
	const FVector ForwardVec    = Observer->GetActorForwardVector();
	const FVector ToTargetNorm  = ToTarget.GetSafeNormal();
	const float   DotProduct    = FVector::DotProduct(ForwardVec, ToTargetNorm);
	const float   HalfAngleRad  = FMath::DegreesToRadians(ConeHalfAngleDeg);
	if (DotProduct < FMath::Cos(HalfAngleRad)) return false;

	// --- Line-of-sight trace ---
	if (bRequireLineOfSight)
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VisionConeTrace), false);
		Params.AddIgnoredActor(Observer);
		Params.AddIgnoredActor(Target);

		// ECC_Visibility respects StaticMesh, BSP, and landscape blocking volumes.
		if (Observer->GetWorld()->LineTraceSingleByChannel(
				Hit, ObserverLoc, TargetLoc, ECC_Visibility, Params))
		{
			// Something blocked the sightline.
			return false;
		}
	}

	return true;
}

bool UVisionConeUtils::IsLocationInVisionCone(
	const AActor* Observer,
	FVector TargetLocation,
	float ConeHalfAngleDeg,
	float MaxRange)
{
	if (!Observer) return false;

	const FVector ObserverLoc = Observer->GetActorLocation();
	const FVector ToTarget    = TargetLocation - ObserverLoc;

	if (ToTarget.SizeSquared() > FMath::Square(MaxRange)) return false;

	const FVector ForwardVec   = Observer->GetActorForwardVector();
	const float   DotProduct   = FVector::DotProduct(ForwardVec, ToTarget.GetSafeNormal());
	const float   HalfAngleRad = FMath::DegreesToRadians(ConeHalfAngleDeg);

	return DotProduct >= FMath::Cos(HalfAngleRad);
}
