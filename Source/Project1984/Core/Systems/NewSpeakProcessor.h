#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NewSpeakProcessor.generated.h"

/**
 * ENewspeakCategory
 *
 * Newspeak vocabulary categories as defined by Orwell.
 */
UENUM(BlueprintType)
enum class ENewspeakCategory : uint8
{
	/** A Vocabulary — everyday words, stripped of secondary meanings */
	AVocabulary		UMETA(DisplayName = "A Vocabulary"),
	/** B Vocabulary — compound words for political purposes (e.g., goodthink, crimestop) */
	BVocabulary		UMETA(DisplayName = "B Vocabulary"),
	/** C Vocabulary — scientific/technical terms */
	CVocabulary		UMETA(DisplayName = "C Vocabulary")
};

/**
 * UNewSpeakProcessor
 *
 * Educational mechanic teaching Orwell's constructed language.
 * Handles voice recognition for spoken Newspeak phrases, written
 * Newspeak puzzles, and Oldspeak-to-Newspeak translation mini-games.
 */
UCLASS()
class PROJECT1984_API UNewSpeakProcessor : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UNewSpeakProcessor();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Validate if a word is proper Newspeak */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	bool IsValidNewspeak(const FString& Word) const;

	/** Translate an Oldspeak phrase to Newspeak */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	FString TranslateToNewspeak(const FString& OldspeakPhrase) const;

	/** Translate a Newspeak phrase to Oldspeak (for educational display) */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	FString TranslateToOldspeak(const FString& NewspeakPhrase) const;

	/** Score how well the player performed a Newspeak challenge */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	float ScoreNewspeakAttempt(const FString& Expected, const FString& PlayerInput) const;

	/** Process voice recognition result for Newspeak validation */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	void ProcessVoiceInput(const FString& RecognizedText);

	/** Get a random Newspeak translation challenge */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	void GetTranslationChallenge(FString& OutOldspeak, FString& OutExpectedNewspeak) const;

protected:
	/** Dictionary mapping Oldspeak to Newspeak equivalents */
	UPROPERTY()
	TMap<FString, FString> NewspeakDictionary;

	/** Load the Newspeak dictionary data */
	void LoadDictionary();
};
