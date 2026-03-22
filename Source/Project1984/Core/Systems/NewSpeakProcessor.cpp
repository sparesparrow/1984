#include "NewSpeakProcessor.h"

UNewSpeakProcessor::UNewSpeakProcessor()
{
}

void UNewSpeakProcessor::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadDictionary();
}

bool UNewSpeakProcessor::IsValidNewspeak(const FString& Word) const
{
	// TODO: Check against Newspeak dictionary
	// Valid Newspeak examples: goodthink, crimestop, doublethink, blackwhite, ownlife
	// Invalid (Oldspeak): freedom, justice, democracy, science, honor
	return NewspeakDictionary.Contains(Word.ToLower());
}

FString UNewSpeakProcessor::TranslateToNewspeak(const FString& OldspeakPhrase) const
{
	// TODO: Implement word-by-word translation with Newspeak grammar rules
	// Example: "very good" → "plusgood", "extremely good" → "doubleplusgood"
	// Example: "bad" → "ungood", "very bad" → "plusungood"
	return OldspeakPhrase;
}

FString UNewSpeakProcessor::TranslateToOldspeak(const FString& NewspeakPhrase) const
{
	// TODO: Reverse translation for educational display
	// Example: "crimethink" → "thoughts or ideas contrary to Party doctrine"
	return NewspeakPhrase;
}

float UNewSpeakProcessor::ScoreNewspeakAttempt(const FString& Expected, const FString& PlayerInput) const
{
	// TODO: Fuzzy matching with generous thresholds (per plan)
	// Phoneme-based matching to account for voice recognition imprecision
	if (Expected.Equals(PlayerInput, ESearchCase::IgnoreCase))
	{
		return 1.0f;
	}
	return 0.0f;
}

void UNewSpeakProcessor::ProcessVoiceInput(const FString& RecognizedText)
{
	// TODO: Integration with Meta Voice SDK
	// TODO: Validate spoken phrase against expected Newspeak
	// TODO: Report to SurveillanceSystem: speaking Newspeak correctly reduces suspicion,
	//       speaking Oldspeak increases it
}

void UNewSpeakProcessor::GetTranslationChallenge(FString& OutOldspeak, FString& OutExpectedNewspeak) const
{
	// TODO: Select a random entry from the dictionary as a challenge
	// TODO: Scale difficulty based on current narrative act
	OutOldspeak = TEXT("freedom");
	OutExpectedNewspeak = TEXT("crimethink");
}

void UNewSpeakProcessor::LoadDictionary()
{
	// TODO: Load Newspeak dictionary from data asset or JSON file
	// Core B-Vocabulary entries from Orwell's appendix:
	NewspeakDictionary.Add(TEXT("goodthink"), TEXT("orthodoxy"));
	NewspeakDictionary.Add(TEXT("crimethink"), TEXT("thoughtcrime"));
	NewspeakDictionary.Add(TEXT("crimestop"), TEXT("protective stupidity"));
	NewspeakDictionary.Add(TEXT("doublethink"), TEXT("reality control"));
	NewspeakDictionary.Add(TEXT("blackwhite"), TEXT("loyal acceptance of contradictions"));
	NewspeakDictionary.Add(TEXT("ownlife"), TEXT("individualism"));
	NewspeakDictionary.Add(TEXT("plusgood"), TEXT("very good"));
	NewspeakDictionary.Add(TEXT("doubleplusgood"), TEXT("excellent"));
	NewspeakDictionary.Add(TEXT("ungood"), TEXT("bad"));
	NewspeakDictionary.Add(TEXT("plusungood"), TEXT("very bad"));
}
