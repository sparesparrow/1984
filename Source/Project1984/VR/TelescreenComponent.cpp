#include "TelescreenComponent.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Audio/SurveillanceAudioManager.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// Propaganda slogans cycled during the broadcast state
static const TCHAR* PropagandaSlogans[] = {
	TEXT("WAR IS PEACE"),
	TEXT("FREEDOM IS SLAVERY"),
	TEXT("IGNORANCE IS STRENGTH"),
	TEXT("BIG BROTHER IS WATCHING YOU"),
	TEXT("2 + 2 = 5"),
	TEXT("OCEANIA HAS ALWAYS BEEN AT WAR WITH EASTASIA"),
};
static const int32 NumSlogans = UE_ARRAY_COUNT(PropagandaSlogans);

UTelescreenComponent::UTelescreenComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	State                    = ETelescreenState::Broadcasting;
	VisionConeAngle          = 60.0f;
	SurveillanceRange        = 800.0f;
	bPlayerInView            = false;
	ObservationReportInterval = 5.0f;
	ObservationTimer         = 0.0f;
	AddressingTimer          = 0.0f;
	bParticipatedInHate      = false;
	PropagandaIndex          = 0;
	SurveillanceSystem       = nullptr;
	AudioManager             = nullptr;
}

void UTelescreenComponent::BeginPlay()
{
	Super::BeginPlay();

	// Resolve subsystems
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
		AudioManager       = GI->GetSubsystem<USurveillanceAudioManager>();
	}

	// Register this telescreen as a surveillance source
	if (SurveillanceSystem && GetOwner())
	{
		SurveillanceSystem->RegisterSurveillanceActor(GetOwner());
	}

	// Log that this telescreen is active (audio/video asset playback
	// is handled Blueprint-side via MediaPlayer once content is created)
	UE_LOG(LogTemp, Log, TEXT("TelescreenComponent: Active on '%s'. Broadcasting propaganda."),
		GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UTelescreenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State != ETelescreenState::Off && State != ETelescreenState::Obscured)
	{
		UpdateSurveillance(DeltaTime);
		UpdateBroadcast(DeltaTime);
	}
}

void UTelescreenComponent::StartTwoMinutesHate()
{
	State               = ETelescreenState::TwoMinutesHate;
	bParticipatedInHate = false;

	// Notify audio manager to switch to hate sequence
	if (AudioManager)
	{
		AudioManager->StartTwoMinutesHateAudio();
	}

	UE_LOG(LogTemp, Log, TEXT("TelescreenComponent: Two Minutes Hate STARTED."));

	// Participation check fires after 120 seconds (2 minutes)
	FTimerHandle HateEndHandle;
	GetWorld()->GetTimerManager().SetTimer(HateEndHandle, this,
		&UTelescreenComponent::EndTwoMinutesHate, 120.0f, /*bLoop=*/false);
}

void UTelescreenComponent::EndTwoMinutesHate()
{
	// Non-participation is suspicious — a loyal citizen always takes part
	if (!bParticipatedInHate && SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::FacialExpression,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::FacialExpression));
		UE_LOG(LogTemp, Warning,
			TEXT("TelescreenComponent: Player did not participate in Two Minutes Hate — suspicion raised."));
	}
	else if (bParticipatedInHate && SurveillanceSystem)
	{
		// Participation reduces suspicion
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::AttendingTwoMinutesHate,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::AttendingTwoMinutesHate));
	}

	State = ETelescreenState::Broadcasting;

	if (AudioManager)
	{
		AudioManager->StopTwoMinutesHateAudio();
	}

	UE_LOG(LogTemp, Log, TEXT("TelescreenComponent: Two Minutes Hate ENDED. Returning to broadcast."));
}

void UTelescreenComponent::ObscureTelescreen()
{
	if (State == ETelescreenState::Broadcasting || State == ETelescreenState::Addressing)
	{
		State = ETelescreenState::Obscured;

		// Obscuring a telescreen is a serious thought crime — high suspicion cost
		if (SurveillanceSystem)
		{
			SurveillanceSystem->ReportIncident(
				ESuspicionEvent::EnteringOffLimits,
				USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::EnteringOffLimits));
		}

		UE_LOG(LogTemp, Warning,
			TEXT("TelescreenComponent: Telescreen OBSCURED on '%s' — suspicion raised."),
			GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));

		// Auto-restore after 30 seconds
		GetWorld()->GetTimerManager().SetTimer(
			RestoreTimerHandle, this,
			&UTelescreenComponent::RestoreFromObscured, 30.0f, /*bLoop=*/false);
	}
}

void UTelescreenComponent::RestoreFromObscured()
{
	if (State == ETelescreenState::Obscured)
	{
		State = ETelescreenState::Broadcasting;
		UE_LOG(LogTemp, Log, TEXT("TelescreenComponent: Telescreen auto-restored on '%s'."),
			GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
	}
}

bool UTelescreenComponent::IsPlayerVisible() const
{
	if (!GetOwner())
	{
		return false;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		return false;
	}

	// Telescreen faces along its owner's forward vector
	const FVector ScreenLocation = GetOwner()->GetActorLocation();
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const FVector ToPlayer       = PlayerLocation - ScreenLocation;
	const float   Distance       = ToPlayer.Size();

	// Distance check
	if (Distance > SurveillanceRange)
	{
		return false;
	}

	// Angle check — dot product against screen forward
	const FVector ToPlayerNorm   = ToPlayer.GetSafeNormal();
	const FVector ScreenForward  = GetOwner()->GetActorForwardVector();
	const float   CosHalfAngle   = FMath::Cos(FMath::DegreesToRadians(VisionConeAngle * 0.5f));
	if (FVector::DotProduct(ScreenForward, ToPlayerNorm) < CosHalfAngle)
	{
		return false;
	}

	// Line-of-sight trace
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(PlayerPawn);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		ScreenLocation,
		PlayerLocation + FVector(0.f, 0.f, 60.f),
		ECC_Visibility,
		Params);

	return !bBlocked;
}

void UTelescreenComponent::UpdateSurveillance(float DeltaTime)
{
	const bool bWasVisible = bPlayerInView;
	bPlayerInView          = IsPlayerVisible();

	if (bPlayerInView)
	{
		// Accumulate observation timer and report periodically
		ObservationTimer += DeltaTime;
		if (ObservationTimer >= ObservationReportInterval)
		{
			ObservationTimer = 0.0f;
			if (SurveillanceSystem)
			{
				SurveillanceSystem->ReportIncident(
					ESuspicionEvent::TelescreenObservation,
					USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::TelescreenObservation));
			}

			// Occasionally switch to Addressing state (Big Brother addresses player directly)
			if (State == ETelescreenState::Broadcasting && FMath::RandBool())
			{
				State          = ETelescreenState::Addressing;
				AddressingTimer = 0.0f;
				UE_LOG(LogTemp, Log, TEXT("TelescreenComponent: Switching to ADDRESSING state."));
			}
		}
	}
	else
	{
		ObservationTimer = 0.0f;
	}

	// Return from Addressing state after 8 seconds
	if (State == ETelescreenState::Addressing)
	{
		AddressingTimer += DeltaTime;
		if (AddressingTimer >= 8.0f)
		{
			State = ETelescreenState::Broadcasting;
		}
	}

	if (bPlayerInView && !bWasVisible)
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("TelescreenComponent: Player entered surveillance view of '%s'."),
			GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
	}
}

void UTelescreenComponent::UpdateBroadcast(float DeltaTime)
{
	// Rotate propaganda content every 15 seconds (Blueprint MediaPlayer drives visuals;
	// we drive the logical content selection here so it can drive text widgets, captions etc.)
	static float BroadcastCycleTimer = 0.0f;
	BroadcastCycleTimer += DeltaTime;
	if (BroadcastCycleTimer >= 15.0f)
	{
		BroadcastCycleTimer = 0.0f;
		PropagandaIndex     = (PropagandaIndex + 1) % NumSlogans;
		UE_LOG(LogTemp, Verbose, TEXT("TelescreenComponent: Now broadcasting — \"%s\""),
			PropagandaSlogans[PropagandaIndex]);

		// Play audio at this telescreen's world location
		if (AudioManager && GetOwner())
		{
			AudioManager->PlayTelescreenAudio(
				GetOwner()->GetActorLocation(),
				FString(PropagandaSlogans[PropagandaIndex]));
		}
	}
}
