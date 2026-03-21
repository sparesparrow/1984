#include "DoublethinkWidget.h"

void UDoublethinkWidget::SetPartyTruth(const FString& PartyVersion)
{
	PartyTruth = PartyVersion;
	// TODO: Update left-hand world-space widget text
	// TODO: Apply Party visual styling (red, bold, authoritative)
}

void UDoublethinkWidget::SetHistoricalTruth(const FString& HistoricalVersion)
{
	HistoricalTruth = HistoricalVersion;
	// TODO: Update right-hand world-space widget text
	// TODO: Apply historical visual styling (faded, handwritten, fragile)
}

void UDoublethinkWidget::SetDissonanceScore(float Score)
{
	DissonanceScore = FMath::Clamp(Score, 0.0f, 1.0f);
	// TODO: Visual feedback — higher dissonance causes screen distortion
}

void UDoublethinkWidget::AcceptPartyTruth()
{
	// TODO: Report to NarrativeManager — player chose conformity
	// TODO: Reduce suspicion slightly (player demonstrated orthodoxy)
	// TODO: Play conformity audio cue
	OnDoublethinkChoice.Broadcast(TEXT("Party"), DissonanceScore);
}

void UDoublethinkWidget::AcceptHistoricalTruth()
{
	// TODO: Report to NarrativeManager — player chose resistance
	// TODO: Increase suspicion (player rejected Party truth)
	// TODO: Play resistance audio cue
	OnDoublethinkChoice.Broadcast(TEXT("Historical"), DissonanceScore);
}

void UDoublethinkWidget::AcceptBothTruths()
{
	// TODO: The hardest choice — true doublethink
	// TODO: Report to NarrativeManager — player achieved cognitive dissonance
	// TODO: Unique visual/audio effect for holding contradictions
	// TODO: Educational moment — explain what doublethink means
	OnDoublethinkChoice.Broadcast(TEXT("Doublethink"), DissonanceScore);
}
