#include "Project1984GameState.h"

AProject1984GameState::AProject1984GameState()
{
	GlobalSuspicionLevel = 0.0f;
	ThoughtcrimeCount = 0;
	bJuliaContactMade = false;
	bOBrienMeetingComplete = false;
}

void AProject1984GameState::RecordDecision(const FString& DecisionID, const FString& Choice)
{
	DecisionLog.Add(DecisionID, Choice);

	// TODO: Broadcast decision event for NarrativeManager to process
	// TODO: Update suspicion level if decision is incriminating
}

FString AProject1984GameState::ExportDecisionLogJSON() const
{
	// TODO: Serialize DecisionLog to JSON format for educational export
	// Format: { "decisions": [ { "id": "...", "choice": "...", "timestamp": "..." } ] }
	FString JSON = TEXT("{\"decisions\": []}");
	return JSON;
}
