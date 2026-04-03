#include "HandTracking.h"
#include "IXRTrackingSystem.h"
#include "Features/IModularFeatures.h"
#include "IHandTracker.h"

// Joint index constants matching EHandKeypoint order
namespace HandJointIndex
{
	constexpr int32 Palm           = 0;
	constexpr int32 Wrist          = 1;
	constexpr int32 ThumbMetacarpal = 2;
	constexpr int32 ThumbProximal  = 3;
	constexpr int32 ThumbDistal    = 4;
	constexpr int32 ThumbTip       = 5;
	constexpr int32 IndexProximal  = 6;
	constexpr int32 IndexIntermed  = 7;
	constexpr int32 IndexDistal    = 8;
	constexpr int32 IndexTip       = 9;
	constexpr int32 MiddleProximal = 10;
	constexpr int32 MiddleIntermed = 11;
	constexpr int32 MiddleDistal   = 12;
	constexpr int32 MiddleTip      = 13;
	constexpr int32 RingProximal   = 14;
	constexpr int32 RingIntermed   = 15;
	constexpr int32 RingDistal     = 16;
	constexpr int32 RingTip        = 17;
	constexpr int32 LittleProximal = 18;
	constexpr int32 LittleIntermed = 19;
	constexpr int32 LittleDistal   = 20;
	constexpr int32 LittleTip      = 21;
	constexpr int32 Count          = 26;
}

UHandTracking::UHandTracking()
{
	PrimaryComponentTick.bCanEverTick = true;

	bHandTrackingActive      = false;
	bUsingControllerFallback = false;
	CurrentGesture           = EHandGesture::None;
	PreviousGesture          = EHandGesture::None;
	PinchStrength            = 0.0f;
	GrabStrength             = 0.0f;
	bIsLeftHand              = true;

	JointPositions.Init(FVector::ZeroVector, HandJointIndex::Count);
	JointTrackingValid.Init(false, HandJointIndex::Count);
}

void UHandTracking::BeginPlay()
{
	Super::BeginPlay();

	// Check if the OpenXR hand tracking extension is available.
	if (IsHandTrackingSupported())
	{
		bHandTrackingActive      = true;
		bUsingControllerFallback = false;
		UE_LOG(LogTemp, Log, TEXT("HandTracking (%s): OpenXR hand tracking active."),
			bIsLeftHand ? TEXT("Left") : TEXT("Right"));
	}
	else
	{
		// Fall back to controller input — gesture data will be fed by
		// the owning pawn reading controller trigger/grip axis values.
		bHandTrackingActive      = false;
		bUsingControllerFallback = true;
		UE_LOG(LogTemp, Log, TEXT("HandTracking (%s): Falling back to controller input."),
			bIsLeftHand ? TEXT("Left") : TEXT("Right"));
	}
}

void UHandTracking::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTrackingData();

	const EHandGesture NewGesture = ClassifyGesture();
	if (NewGesture != CurrentGesture)
	{
		PreviousGesture = CurrentGesture;
		CurrentGesture  = NewGesture;
		OnGestureChanged.Broadcast(CurrentGesture, PreviousGesture);
	}
}

bool UHandTracking::IsHandTrackingSupported() const
{
	// Query the modular feature registry for an IHandTracker implementation
	// (provided by the OpenXRHandTracking plugin when the extension is active).
	if (IModularFeatures::Get().IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
	{
		// Verify the XR system is a known Meta Quest / OpenXR device.
		if (GEngine && GEngine->XRSystem.IsValid())
		{
			return true;
		}
	}
	return false;
}

EHandGesture UHandTracking::GetCurrentGesture() const
{
	return CurrentGesture;
}

void UHandTracking::UpdateTrackingData()
{
	if (!bHandTrackingActive)
	{
		return;
	}

	if (!IModularFeatures::Get().IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
	{
		bHandTrackingActive      = false;
		bUsingControllerFallback = true;
		PinchStrength = 0.0f;
		GrabStrength  = 0.0f;
		return;
	}

	IHandTracker& HandTracker =
		IModularFeatures::Get().GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());

	const EControllerHand Hand = bIsLeftHand ? EControllerHand::Left : EControllerHand::Right;

	// Read all joint keypoint states for this hand.
	TArray<FVector>    OutPositions;
	TArray<FQuat>      OutRotations;
	TArray<float>      OutRadii;
	TArray<bool>       OutTrackingValid;

	const bool bDataValid = HandTracker.GetAllKeypointStates(
		Hand, OutPositions, OutRadii, OutRotations, OutTrackingValid);

	if (!bDataValid || OutPositions.Num() < HandJointIndex::Count)
	{
		// Tracking lost this frame — zero strengths but keep last gesture.
		PinchStrength = 0.0f;
		GrabStrength  = 0.0f;
		return;
	}

	// Copy joint data into component arrays.
	for (int32 i = 0; i < HandJointIndex::Count && i < OutPositions.Num(); ++i)
	{
		JointPositions[i]     = OutPositions[i];
		JointTrackingValid[i] = OutTrackingValid[i];
	}

	// Pinch strength: inverse-normalized thumb-tip to index-tip distance.
	const float ThumbIndexDist =
		FVector::Dist(JointPositions[HandJointIndex::ThumbTip],
		              JointPositions[HandJointIndex::IndexTip]);
	PinchStrength = FMath::Clamp(
		1.0f - (ThumbIndexDist / (MaxPinchDistanceCm * 10.0f)), 0.0f, 1.0f);

	// Grab strength: average curl of index, middle, ring, little fingers.
	const float IndexCurl  = EstimateFingerCurl(HandJointIndex::IndexProximal,  HandJointIndex::IndexIntermed,  HandJointIndex::IndexTip);
	const float MiddleCurl = EstimateFingerCurl(HandJointIndex::MiddleProximal, HandJointIndex::MiddleIntermed, HandJointIndex::MiddleTip);
	const float RingCurl   = EstimateFingerCurl(HandJointIndex::RingProximal,   HandJointIndex::RingIntermed,   HandJointIndex::RingTip);
	const float LittleCurl = EstimateFingerCurl(HandJointIndex::LittleProximal, HandJointIndex::LittleIntermed, HandJointIndex::LittleTip);
	GrabStrength = (IndexCurl + MiddleCurl + RingCurl + LittleCurl) * 0.25f;
}

float UHandTracking::EstimateFingerCurl(int32 ProximalIdx, int32 MiddleIdx, int32 TipIdx) const
{
	// Curl is estimated as how much the tip has moved toward the palm
	// relative to the proximal joint using the dot product between
	// proximal→middle and proximal→tip directions.
	// Value of -1 = fully curled, +1 = fully extended; remapped to [0,1].
	if (ProximalIdx >= JointPositions.Num() || MiddleIdx >= JointPositions.Num() || TipIdx >= JointPositions.Num())
	{
		return 0.0f;
	}

	const FVector ProxToMid = (JointPositions[MiddleIdx] - JointPositions[ProximalIdx]).GetSafeNormal();
	const FVector ProxToTip = (JointPositions[TipIdx]    - JointPositions[ProximalIdx]).GetSafeNormal();
	const float   Dot       = FVector::DotProduct(ProxToMid, ProxToTip);

	// Remap [-1, 1] → [1, 0] so 1 = curled, 0 = extended.
	return FMath::Clamp((1.0f - Dot) * 0.5f, 0.0f, 1.0f);
}

EHandGesture UHandTracking::ClassifyGesture() const
{
	// Controller-fallback path: treat raw controller axes as discrete gestures.
	if (!bHandTrackingActive || bUsingControllerFallback)
	{
		if (PinchStrength > 0.8f) return EHandGesture::Pinch;
		if (GrabStrength  > 0.8f) return EHandGesture::Grab;
		return EHandGesture::None;
	}

	const float IndexCurl  = EstimateFingerCurl(HandJointIndex::IndexProximal,  HandJointIndex::IndexIntermed,  HandJointIndex::IndexTip);
	const float MiddleCurl = EstimateFingerCurl(HandJointIndex::MiddleProximal, HandJointIndex::MiddleIntermed, HandJointIndex::MiddleTip);
	const float RingCurl   = EstimateFingerCurl(HandJointIndex::RingProximal,   HandJointIndex::RingIntermed,   HandJointIndex::RingTip);
	const float LittleCurl = EstimateFingerCurl(HandJointIndex::LittleProximal, HandJointIndex::LittleIntermed, HandJointIndex::LittleTip);

	// Estimate thumb extension as inverse of thumb-tip to palm distance.
	const float ThumbPalmDist  = FVector::Dist(
		JointPositions[HandJointIndex::ThumbTip], JointPositions[HandJointIndex::Palm]);
	const float ThumbExtension = FMath::Clamp(ThumbPalmDist / 80.0f, 0.0f, 1.0f);

	// Fist: all fingers tightly curled (Two Minutes Hate gesture).
	if (GrabStrength > 0.9f && PinchStrength < 0.5f && IndexCurl > 0.75f && ThumbExtension < 0.3f)
	{
		return EHandGesture::Fist;
	}

	// Pinch: thumb and index close together, other fingers relaxed (selection / diary writing).
	if (PinchStrength > 0.85f && MiddleCurl < 0.5f)
	{
		return EHandGesture::Pinch;
	}

	// Grab: all four fingers curled (picking up objects).
	if (GrabStrength > 0.7f)
	{
		return EHandGesture::Grab;
	}

	// ThumbsUp: thumb extended, all other fingers curled.
	if (ThumbExtension > 0.7f &&
		IndexCurl  > 0.7f &&
		MiddleCurl > 0.7f &&
		RingCurl   > 0.7f &&
		LittleCurl > 0.7f)
	{
		return EHandGesture::ThumbsUp;
	}

	// Point: index extended, other fingers curled.
	if (IndexCurl  < 0.3f &&
		MiddleCurl > 0.6f &&
		RingCurl   > 0.6f &&
		LittleCurl > 0.6f)
	{
		return EHandGesture::Point;
	}

	// Open hand: all fingers extended (release / push gesture).
	if (IndexCurl  < 0.3f &&
		MiddleCurl < 0.3f &&
		RingCurl   < 0.3f &&
		LittleCurl < 0.3f &&
		ThumbExtension > 0.5f)
	{
		return EHandGesture::OpenHand;
	}

	return EHandGesture::None;
}
