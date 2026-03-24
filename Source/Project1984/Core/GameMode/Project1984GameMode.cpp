#include "Project1984GameMode.h"
#include "Project1984/VR/VRPawnBase.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Systems/NarrativeManager.h"
#include "Project1984/Audio/SurveillanceAudioManager.h"

AProject1984GameMode::AProject1984GameMode()
{
	DefaultPawnClass    = AVRPawnBase::StaticClass();
	CurrentAct          = 1;
	bPlayerCaptured     = false;
	SurveillanceSystem  = nullptr;
	NarrativeManager    = nullptr;
}

void AProject1984GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	InitializeSubsystems();

	// Save-game restoration: persisted act and suspicion state loaded here once
	// USaveGame subclass is implemented (Phase 3). For now, subsystems initialize
	// from defaults.

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameMode: InitGame — Map='%s' Act=%d SS=%s NM=%s"),
		*MapName, CurrentAct,
		SurveillanceSystem ? TEXT("OK") : TEXT("null"),
		NarrativeManager   ? TEXT("OK") : TEXT("null"));
}

void AProject1984GameMode::StartPlay()
{
	Super::StartPlay();

	// Bind to NarrativeManager act-transition delegate so GameMode stays in sync
	if (NarrativeManager)
	{
		NarrativeManager->OnActChanged.AddDynamic(
			this, &AProject1984GameMode::OnNarrativeActChanged);

		// Sync CurrentAct in case a saved game was already applied to NarrativeManager
		CurrentAct = static_cast<int32>(NarrativeManager->CurrentAct) + 1;
	}

	if (SurveillanceSystem)
	{
		UE_LOG(LogTemp, Log,
			TEXT("Project1984GameMode: StartPlay — surveillance active. Act %d begins."),
			CurrentAct);
	}

	// Play opening telescreen morning broadcast via audio subsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USurveillanceAudioManager* Audio = GI->GetSubsystem<USurveillanceAudioManager>())
		{
			Audio->PlayTelescreenAudio(FVector::ZeroVector, TEXT("MorningBroadcast"));
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameMode: StartPlay complete — Act %d. Big Brother is watching."),
		CurrentAct);
}

int32 AProject1984GameMode::GetCurrentAct() const
{
	return CurrentAct;
}

void AProject1984GameMode::AdvanceAct()
{
	if (!NarrativeManager)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Project1984GameMode: AdvanceAct called but NarrativeManager is null."));
		return;
	}

	if (!NarrativeManager->CanAdvanceAct())
	{
		UE_LOG(LogTemp, Log,
			TEXT("Project1984GameMode: AdvanceAct — conditions not met for Act %d → %d. "
			     "Check NarrativeManager completion flags."),
			CurrentAct, CurrentAct + 1);
		return;
	}

	const ENarrativeAct PreviousAct = NarrativeManager->CurrentAct;
	NarrativeManager->AdvanceAct();
	CurrentAct = static_cast<int32>(NarrativeManager->CurrentAct) + 1;

	if (NarrativeManager->CurrentAct == ENarrativeAct::CaptureConditioning)
	{
		bPlayerCaptured = true;
		UE_LOG(LogTemp, Warning,
			TEXT("Project1984GameMode: Player captured — Act IV (Capture & Conditioning) begins."));
	}

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameMode: AdvanceAct — %d → Act %d"),
		static_cast<int32>(PreviousAct) + 1, CurrentAct);
}

bool AProject1984GameMode::IsPlayerCaptured() const
{
	return bPlayerCaptured;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void AProject1984GameMode::InitializeSubsystems()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Project1984GameMode: InitializeSubsystems — no GameInstance!"));
		return;
	}

	SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
	NarrativeManager   = GI->GetSubsystem<UNarrativeManager>();

	if (!SurveillanceSystem)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Project1984GameMode: SurveillanceSystem subsystem not found!"));
	}

	if (!NarrativeManager)
	{
		UE_LOG(LogTemp, Error,
			TEXT("Project1984GameMode: NarrativeManager subsystem not found!"));
	}

	if (NarrativeManager)
	{
		CurrentAct = static_cast<int32>(NarrativeManager->CurrentAct) + 1;
	}

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameMode: Subsystems initialized — SS=%s NM=%s Act=%d"),
		SurveillanceSystem ? TEXT("OK") : TEXT("null"),
		NarrativeManager   ? TEXT("OK") : TEXT("null"),
		CurrentAct);
}

void AProject1984GameMode::OnNarrativeActChanged(ENarrativeAct NewAct, ENarrativeAct PreviousAct)
{
	CurrentAct = static_cast<int32>(NewAct) + 1;

	if (NewAct == ENarrativeAct::CaptureConditioning)
	{
		bPlayerCaptured = true;
	}

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameMode: Act transition — Act %d → Act %d"),
		static_cast<int32>(PreviousAct) + 1,
		CurrentAct);
}
