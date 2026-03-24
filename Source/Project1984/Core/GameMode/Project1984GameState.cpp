#include "Project1984GameState.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"

AProject1984GameState::AProject1984GameState()
{
	GlobalSuspicionLevel   = 0.0f;
	ThoughtcrimeCount      = 0;
	bJuliaContactMade      = false;
	bOBrienMeetingComplete = false;
	SurveillanceSystem     = nullptr;
}

void AProject1984GameState::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameState: Initialized. Decision logging active. SS=%s"),
		SurveillanceSystem ? TEXT("OK") : TEXT("null"));
}

void AProject1984GameState::RecordDecision(const FString& DecisionID, const FString& Choice)
{
	DecisionLog.Add(DecisionID, Choice);

	// Count thoughtcrimes — choices indicating resistance or illegal acts
	if (Choice.Contains(TEXT("Resist"))    ||
	    Choice.Contains(TEXT("Diary"))     ||
	    Choice.Contains(TEXT("Historical")) ||
	    Choice.Contains(TEXT("Julia"))     ||
	    Choice.Contains(TEXT("OBrien")))
	{
		ThoughtcrimeCount++;
	}

	// Mirror to SurveillanceSystem decision history for debrief and ending logic
	if (SurveillanceSystem)
	{
		SurveillanceSystem->DecisionHistory.Add(
			FString::Printf(TEXT("[Decision] %s: %s"), *DecisionID, *Choice));
	}

	OnDecisionRecorded.Broadcast(DecisionID, Choice);

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameState: Decision recorded — '%s': '%s'  (ThoughtcrimeCount=%d)"),
		*DecisionID, *Choice, ThoughtcrimeCount);
}

FString AProject1984GameState::ExportDecisionLogJSON() const
{
	FString JSON = TEXT("{\"decisions\":[");
	bool bFirst  = true;

	for (const TPair<FString, FString>& Entry : DecisionLog)
	{
		if (!bFirst)
		{
			JSON += TEXT(",");
		}
		JSON += FString::Printf(
			TEXT("{\"id\":\"%s\",\"choice\":\"%s\"}"),
			*Entry.Key, *Entry.Value);
		bFirst = false;
	}

	JSON += TEXT("]");

	// Append surveillance incident count for educational debrief correlation
	if (SurveillanceSystem)
	{
		JSON += FString::Printf(
			TEXT(",\"incident_count\":%d"),
			SurveillanceSystem->DecisionHistory.Num());
	}

	JSON += FString::Printf(
		TEXT(",\"thoughtcrime_count\":%d"), ThoughtcrimeCount);

	JSON += TEXT("}");

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameState: ExportDecisionLogJSON — %d decisions, %d thoughtcrimes."),
		DecisionLog.Num(), ThoughtcrimeCount);

	return JSON;
}
