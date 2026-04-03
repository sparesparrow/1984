#include "Project1984GameState.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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
	// Overwrite if the same decision key is revisited (last state wins).
	DecisionLog.Add(DecisionID, Choice);

	// Count events that constitute recorded thoughtcrime for the ending determination.
	static const TArray<FString> ThoughtcrimeKeywords = {
		TEXT("WritingDiary"),
		TEXT("SecretMeeting"),
		TEXT("EnteringOffLimits"),
		TEXT("SpeakingOldspeak"),
		TEXT("Resist"),
		TEXT("Historical"),
		TEXT("Julia"),
		TEXT("OBrien"),
	};
	for (const FString& Keyword : ThoughtcrimeKeywords)
	{
		if (DecisionID.Contains(Keyword) || Choice.Contains(Keyword))
		{
			ThoughtcrimeCount++;
			break;
		}
	}

	// Mirror to SurveillanceSystem decision history for debrief and ending logic.
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
	// Build JSON manually — no external dependency required.
	// Format: { "suspicion": 0.42, "thoughtcrimes": 3, "decisions": [...] }
	FString JSON = TEXT("{");
	JSON += FString::Printf(TEXT("\"suspicion\":%.4f,"), GlobalSuspicionLevel);
	JSON += FString::Printf(TEXT("\"thoughtcrimes\":%d,"), ThoughtcrimeCount);
	JSON += TEXT("\"juliaContact\":") + FString(bJuliaContactMade ? TEXT("true") : TEXT("false")) + TEXT(",");
	JSON += TEXT("\"obrienMeeting\":") + FString(bOBrienMeetingComplete ? TEXT("true") : TEXT("false")) + TEXT(",");
	JSON += TEXT("\"decisions\":[");

	bool bFirst = true;
	for (const TPair<FString, FString>& Entry : DecisionLog)
	{
		if (!bFirst) JSON += TEXT(",");
		// Escape any quotes in key/value to keep the JSON valid.
		FString SafeKey   = Entry.Key.Replace(TEXT("\""), TEXT("\\\""));
		FString SafeValue = Entry.Value.Replace(TEXT("\""), TEXT("\\\""));
		JSON += FString::Printf(TEXT("{\"id\":\"%s\",\"choice\":\"%s\"}"), *SafeKey, *SafeValue);
		bFirst = false;
	}

	// Append surveillance incident count for educational debrief correlation.
	if (SurveillanceSystem)
	{
		JSON += FString::Printf(TEXT("],\"incident_count\":%d"),
			SurveillanceSystem->DecisionHistory.Num());
	}
	else
	{
		JSON += TEXT("]");
	}

	JSON += TEXT("}");

	// Write to the project's Saved/ directory for classroom/educator export.
	const FString FilePath = FPaths::ProjectSavedDir() / TEXT("1984_DecisionLog.json");
	if (!FFileHelper::SaveStringToFile(JSON, *FilePath))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("AProject1984GameState: Failed to write decision log to %s"), *FilePath);
	}

	UE_LOG(LogTemp, Log,
		TEXT("Project1984GameState: ExportDecisionLogJSON — %d decisions, %d thoughtcrimes."),
		DecisionLog.Num(), ThoughtcrimeCount);

	return JSON;
}
