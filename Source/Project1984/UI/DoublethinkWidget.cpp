#include "DoublethinkWidget.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Systems/NarrativeManager.h"

void UDoublethinkWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Resolve subsystems via the owning player's game instance
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
		TEXT("DoublethinkWidget: Constructed — awaiting truth presentation. "
		     "SS=%s NM=%s"),
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
	// Blueprint: left-hand UWidgetComponent reads PartyTruth and renders in
	// Ingsoc red with bold Ministry font.
}

void UDoublethinkWidget::SetHistoricalTruth(const FString& HistoricalVersion)
{
	HistoricalTruth = HistoricalVersion;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Historical truth (right hand) — \"%s\"  [faded, handwritten, fragile]"),
		*HistoricalVersion);
	// Blueprint: right-hand widget renders in muted sepia, handwritten font,
	// slightly transparent — representing suppressed memory.
}

void UDoublethinkWidget::SetDissonanceScore(float Score)
{
	DissonanceScore = FMath::Clamp(Score, 0.0f, 1.0f);

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Cognitive dissonance score = %.2f"), DissonanceScore);

	// Blueprint: high dissonance triggers screen-edge distortion / chromatic
	// aberration driven by the DissonanceScore property.

	// High dissonance causes involuntary facial tells that telescreens can detect
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

	// Conformity is orthodoxy: small suspicion reduction (same weight as attending hate)
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::AttendingTwoMinutesHate,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::AttendingTwoMinutesHate));
	}

	// Log conformity to decision history
	if (SurveillanceSystem)
	{
		const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		SurveillanceSystem->DecisionHistory.Add(
			FString::Printf(TEXT("[%.2fs] Doublethink — chose PARTY TRUTH: \"%s\""),
				T, *PartyTruth));
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

	// Rejecting Party truth is a thought crime — same weight as writing in the diary
	if (SurveillanceSystem)
	{
		SurveillanceSystem->ReportIncident(
			ESuspicionEvent::WritingDiary,
			USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::WritingDiary));
	}

	// Log resistance choice — contributes toward non-TotalCompliance ending
	if (SurveillanceSystem)
	{
		const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		SurveillanceSystem->DecisionHistory.Add(
			FString::Printf(TEXT("[%.2fs] Doublethink — chose HISTORICAL TRUTH (resistance): \"%s\""),
				T, *HistoricalTruth));
	}

	// Flag that the player has demonstrated inner resistance (helps DetermineEnding)
	// The NarrativeManager's ending logic already checks bOBrienContacted and suspicion;
	// here we also record the choice so Blueprint-driven events can react.

	OnDoublethinkChoice.Broadcast(TEXT("Historical"), DissonanceScore);
}

void UDoublethinkWidget::AcceptBothTruths()
{
	if (bChoiceMade) { return; }
	bChoiceMade = true;

	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: Player achieved TRUE DOUBLETHINK — holding both simultaneously."));

	// Educational moment — explain the concept in the log (also feeds subtitle system)
	UE_LOG(LogTemp, Log,
		TEXT("DoublethinkWidget: [Educational] "
		     "Doublethink — the power of holding two contradictory beliefs, and accepting both. "
		     "In 1984 this is not hypocrisy but genuine, trained unconscious self-deception. "
		     "The Party requires citizens to know the truth and simultaneously deny it. "
		     "Real-world parallel: motivated reasoning and cognitive dissonance in closed societies."));

	// True doublethink is the Party ideal — neither conformity nor resistance registers
	// as suspicious; it is the expected state of an ideal citizen. No suspicion change.
	if (SurveillanceSystem)
	{
		const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		SurveillanceSystem->DecisionHistory.Add(
			FString::Printf(
				TEXT("[%.2fs] Doublethink — HELD BOTH TRUTHS simultaneously (true doublethink). "
				     "Party: \"%s\" / Historical: \"%s\""),
				T, *PartyTruth, *HistoricalTruth));
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
