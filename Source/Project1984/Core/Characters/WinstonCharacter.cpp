#include "WinstonCharacter.h"
#include "Project1984/Core/Systems/SuspicionComponent.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Systems/NarrativeManager.h"
#include "Project1984/Core/GameMode/Project1984GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

AWinstonCharacter::AWinstonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SuspicionComponent   = CreateDefaultSubobject<USuspicionComponent>(TEXT("SuspicionComponent"));
	bIsWritingDiary      = false;
	bHasDiary            = false;
	bIsUnderSurveillance = false;
	FacialTensionScore   = 0.0f;
	SurveillanceSystem   = nullptr;
	NarrativeManager     = nullptr;
}

void AWinstonCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Resolve subsystems.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
		NarrativeManager   = GI->GetSubsystem<UNarrativeManager>();
	}

	// Initialize Winston's starting state based on the current narrative act.
	if (NarrativeManager)
	{
		switch (NarrativeManager->CurrentAct)
		{
		case ENarrativeAct::OrdinaryLife:
			// Act I: Winston doesn't have the diary yet.
			bHasDiary = false;
			UE_LOG(LogTemp, Log, TEXT("WinstonCharacter: Act I — no diary, ordinary life begins."));
			break;

		case ENarrativeAct::GrowingDoubt:
			// Act II: Diary should be acquirable.
			bHasDiary = NarrativeManager->bDiaryAcquired;
			UE_LOG(LogTemp, Log, TEXT("WinstonCharacter: Act II — bDiaryAcquired=%d"), bHasDiary);
			break;

		default:
			// Later acts: diary state is restored from save.
			bHasDiary = NarrativeManager->bDiaryAcquired;
			break;
		}
	}
	else
	{
		// Fallback: try to restore from GameState decision log.
		if (const AProject1984GameState* GS = GetWorld()->GetGameState<AProject1984GameState>())
		{
			const FString* DiaryState = GS->DecisionLog.Find(TEXT("DiaryAcquired"));
			if (DiaryState && *DiaryState == TEXT("true"))
			{
				bHasDiary = true;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WinstonCharacter: Initialized. HasDiary=%d"), bHasDiary);
}

void AWinstonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSurveillanceStatus();
	UpdateFacialTension(DeltaTime);
}

void AWinstonCharacter::StartDiaryWriting()
{
	if (!bHasDiary) return;

	bIsWritingDiary = true;

	// Writing the diary is a thought crime — immediately raise suspicion.
	if (SuspicionComponent)
	{
		SuspicionComponent->ReportSuspicionEvent(ESuspicionEvent::WritingDiary);
	}

	// Mark the diary as acquired in the NarrativeManager if this is first use.
	if (bHasDiary && NarrativeManager && !NarrativeManager->bDiaryAcquired)
	{
		NarrativeManager->bDiaryAcquired = true;
		UE_LOG(LogTemp, Log, TEXT("WinstonCharacter: Diary acquisition flagged in NarrativeManager."));
	}

	// Record the decision so the educator export captures it.
	if (AProject1984GameState* GS = GetWorld()->GetGameState<AProject1984GameState>())
	{
		GS->RecordDecision(TEXT("WritingDiary"), TEXT("started"));
	}

	UE_LOG(LogTemp, Log, TEXT("WinstonCharacter: Started writing diary — suspicion +0.15."));
}

void AWinstonCharacter::StopDiaryWriting()
{
	if (!bIsWritingDiary) return;

	bIsWritingDiary = false;

	// Save diary entry content to the decision log for educational export.
	if (SurveillanceSystem)
	{
		SurveillanceSystem->DecisionHistory.Add(
			FString::Printf(TEXT("[%.2fs] DiaryEntry recorded."),
				GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f));
	}

	if (AProject1984GameState* GS = GetWorld()->GetGameState<AProject1984GameState>())
	{
		GS->RecordDecision(TEXT("WritingDiary"), TEXT("completed"));
	}

	UE_LOG(LogTemp, Log, TEXT("WinstonCharacter: Stopped writing diary. Entry logged."));
}

bool AWinstonCharacter::IsBeingWatched() const
{
	if (!SurveillanceSystem)
	{
		return false;
	}

	// Active surveillance if any source registered and suspicion is above observation threshold.
	return SurveillanceSystem->GetActiveSurveillanceCount() > 0
		&& SurveillanceSystem->GetSuspicionLevel() >= 0.3f;
}

void AWinstonCharacter::UpdateSurveillanceStatus()
{
	if (!GetWorld())
	{
		return;
	}

	bool bWatched = false;

	// Check line-of-sight from nearby TelescreenComponents.
	for (TObjectIterator<class UTelescreenComponent> It; It; ++It)
	{
		if (It->GetWorld() != GetWorld())
		{
			continue;
		}
		if (It->State != ETelescreenState::Off && It->bPlayerInView)
		{
			bWatched = true;
			break;
		}
	}

	// Check proximity to Thought Police NPCs (any NPC with ThoughtPolice loyalty).
	if (!bWatched)
	{
		for (TObjectIterator<class ANPCBase> It; It; ++It)
		{
			if (It->GetWorld() != GetWorld())
			{
				continue;
			}
			if (It->Loyalty == ENPCLoyalty::ThoughtPolice && It->CanSeePlayer())
			{
				bWatched = true;
				break;
			}
		}
	}

	bIsUnderSurveillance = bWatched;

	// Keep SuspicionComponent in sync.
	if (SuspicionComponent)
	{
		SuspicionComponent->bUnderSurveillance = bWatched;
	}
}

void AWinstonCharacter::UpdateFacialTension(float DeltaTime)
{
	// Quest Pro eye-tracking heuristic:
	// When under surveillance, a guilty person averts gaze or blinks rapidly.
	// We approximate this by increasing facial tension while bIsWritingDiary or
	// bIsUnderSurveillance is true. Actual eye-tracking is done in Blueprint.

	const float TargetTension = (bIsWritingDiary || bIsUnderSurveillance) ? 1.0f : 0.0f;
	FacialTensionScore = FMath::FInterpTo(FacialTensionScore, TargetTension, DeltaTime, 0.5f);

	// Report FacialExpression suspicion when tension is high and player is watched.
	static float FacialReportTimer = 0.0f;
	FacialReportTimer += DeltaTime;
	if (FacialTensionScore > 0.7f && bIsUnderSurveillance && FacialReportTimer >= 10.0f)
	{
		FacialReportTimer = 0.0f;
		if (SuspicionComponent)
		{
			SuspicionComponent->ReportSuspicionEvent(ESuspicionEvent::FacialExpression);
		}
	}
}
