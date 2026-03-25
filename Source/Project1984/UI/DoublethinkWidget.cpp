#include "DoublethinkWidget.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Systems/NarrativeManager.h"
#include "Project1984/Core/Systems/SuspicionComponent.h"
#include "Project1984/Core/GameMode/Project1984GameState.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UDoublethinkWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Resolve subsystems via the owning player's game instance.
	UWorld* World = GetWorld();
	if (World)
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
			NarrativeManager   = GI->GetSubsystem<UNarrativeManager>();
		}
	}

	DissonanceScore = 0.0f;
	bChoiceMade     = false;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Constructed — awaiting truth presentation. SS=%s NM=%s"),
		SurveillanceSystem ? TEXT("OK") : TEXT("null"),
		NarrativeManager   ? TEXT("OK") : TEXT("null"));
}

// ---------------------------------------------------------------------------
// Truth setters
// ---------------------------------------------------------------------------

void UDoublethinkWidget::SetPartyTruth(const FString& PartyVersion)
{
	PartyTruth = PartyVersion;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Party truth (left hand) — \"%s\"  [red, bold, authoritative]"),
		*PartyVersion);
}

void UDoublethinkWidget::SetHistoricalTruth(const FString& HistoricalVersion)
{
	HistoricalTruth = HistoricalVersion;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Historical truth (right hand) — \"%s\"  [faded, handwritten, fragile]"),
		*HistoricalVersion);
}

void UDoublethinkWidget::SetDissonanceScore(float Score)
{
	DissonanceScore = FMath::Clamp(Score, 0.0f, 1.0f);

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Cognitive dissonance score = %.2f"), DissonanceScore);

	// High dissonance causes involuntary facial tells that telescreens can detect.
	if (DissonanceScore > 0.7f && SurveillanceSystem)
	{
		const float Weight =
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::FacialExpression)
			* DissonanceScore;

		SurveillanceSystem->ReportIncident(ESuspicionEvent::FacialExpression, Weight);

		UE_LOG(LogTemp, Log,
			TEXT("DoublethinkWidget: Dissonance %.2f — facial micro-expression reported (weight %.3f)."),
			DissonanceScore, Weight);
	}
}

// ---------------------------------------------------------------------------
// Choice handlers
// ---------------------------------------------------------------------------

void UDoublethinkWidget::AcceptPartyTruth()
{
	if (bChoiceMade) { return; }
	bChoiceMade = true;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Player chose CONFORMITY — accepted Party truth \"%s\"."),
		*PartyTruth);

	// Conformity is orthodoxy: small suspicion reduction.
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::AttendingTwoMinutesHate,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::AttendingTwoMinutesHate));
	}

	// Log conformity to decision history.
	if (UWorld* World = GetWorld())
	{
		if (AProject1984GameState* GS = World->GetGameState<AProject1984GameState>())
		{
			GS->RecordDecision(
				FString::Printf(TEXT("Doublethink_%s"), *PartyTruth.Left(32)),
				TEXT("AcceptedPartyTruth"));
		}

		if (SurveillanceSystem)
		{
			const float T = World->GetTimeSeconds();
			SurveillanceSystem->DecisionHistory.Add(
				FString::Printf(TEXT("[%.2fs] Doublethink — chose PARTY TRUTH: \"%s\""),
					T, *PartyTruth));
		}
	}

	OnDoublethinkChoice.Broadcast(TEXT("Party"), DissonanceScore);
}

void UDoublethinkWidget::AcceptHistoricalTruth()
{
	if (bChoiceMade) { return; }
	bChoiceMade = true;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Player chose RESISTANCE — accepted historical truth \"%s\"."),
		*HistoricalTruth);

	// Rejecting Party truth is a thought crime.
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::SpeakingOldspeak, 0.10f);
	}

	// Log resistance choice — contributes toward non-TotalCompliance ending.
	if (UWorld* World = GetWorld())
	{
		if (AProject1984GameState* GS = World->GetGameState<AProject1984GameState>())
		{
			GS->RecordDecision(
				FString::Printf(TEXT("Doublethink_%s"), *PartyTruth.Left(32)),
				TEXT("AcceptedHistoricalTruth"));
		}

		if (SurveillanceSystem)
		{
			const float T = World->GetTimeSeconds();
			SurveillanceSystem->DecisionHistory.Add(
				FString::Printf(TEXT("[%.2fs] Doublethink — chose HISTORICAL TRUTH (resistance): \"%s\""),
					T, *HistoricalTruth));
		}
	}

	OnDoublethinkChoice.Broadcast(TEXT("Historical"), DissonanceScore);
}

void UDoublethinkWidget::AcceptBothTruths()
{
	if (bChoiceMade) { return; }
	bChoiceMade = true;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Player achieved TRUE DOUBLETHINK — holding both simultaneously."));

	// Educational moment — explain the concept in the log.
	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: [Educational] "
		     "Doublethink — the power of holding two contradictory beliefs, and accepting both. "
		     "In 1984 this is not hypocrisy but genuine, trained unconscious self-deception. "
		     "The Party requires citizens to know the truth and simultaneously deny it. "
		     "Real-world parallel: motivated reasoning and cognitive dissonance in closed societies."));

	// True doublethink is the Party ideal — no suspicion change.
	if (UWorld* World = GetWorld())
	{
		if (AProject1984GameState* GS = World->GetGameState<AProject1984GameState>())
		{
			// "Doublethink" keyword is intentionally NOT in the thoughtcrime list —
			// the Party approves. But we record it so educators can discuss it.
			GS->RecordDecision(
				FString::Printf(TEXT("Doublethink_%s"), *PartyTruth.Left(32)),
				TEXT("AcceptedBothTruths_Doublethink"));
		}

		if (SurveillanceSystem)
		{
			const float T = World->GetTimeSeconds();
			SurveillanceSystem->DecisionHistory.Add(
				FString::Printf(
					TEXT("[%.2fs] Doublethink — HELD BOTH TRUTHS simultaneously (true doublethink). "
					     "Party: \"%s\" / Historical: \"%s\""),
					T, *PartyTruth, *HistoricalTruth));
		}
	}

	OnDoublethinkChoice.Broadcast(TEXT("Doublethink"), DissonanceScore);
}

void UDoublethinkWidget::ResetChoice()
{
	bChoiceMade     = false;
	DissonanceScore = 0.0f;
	PartyTruth.Empty();
	HistoricalTruth.Empty();

	UE_LOG(LogTemp, Verbose, TEXT("DoublethinkWidget: Reset for next challenge."));
}
