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
 * FNewspeakEntry
 *
 * A single Oldspeak ↔ Newspeak mapping with educational metadata.
 */
USTRUCT(BlueprintType)
struct FNewspeakEntry
{
	GENERATED_BODY()

	/** The Oldspeak word or phrase being replaced */
	UPROPERTY(BlueprintReadOnly)
	FString Oldspeak;

	/** The Newspeak equivalent */
	UPROPERTY(BlueprintReadOnly)
	FString Newspeak;

	/** Human-readable definition for the educational debrief */
	UPROPERTY(BlueprintReadOnly)
	FString Definition;

	/** Vocabulary category */
	UPROPERTY(BlueprintReadOnly)
	ENewspeakCategory Category;
};

/**
 * UNewSpeakProcessor
 *
 * Educational mechanic teaching Orwell's constructed language.
 * Handles voice recognition for spoken Newspeak phrases, written
 * Newspeak puzzles, and Oldspeak-to-Newspeak translation mini-games.
 *
 * Translation rules follow Orwell's Appendix to Nineteen Eighty-Four.
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

	/** Translate an Oldspeak phrase to Newspeak using grammar rules */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	FString TranslateToNewspeak(const FString& OldspeakPhrase) const;

	/** Translate a Newspeak phrase to Oldspeak (for educational display) */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	FString TranslateToOldspeak(const FString& NewspeakPhrase) const;

	/**
	 * Score how well the player performed a Newspeak challenge.
	 * Returns 0.0 (fail) to 1.0 (perfect) using edit-distance fuzzy matching.
	 */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	float ScoreNewspeakAttempt(const FString& Expected, const FString& PlayerInput) const;

	/** Process voice recognition result for Newspeak validation */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	void ProcessVoiceInput(const FString& RecognizedText);

	/** Get a random Newspeak translation challenge scaled by narrative act index (0–4) */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	void GetTranslationChallenge(int32 ActIndex, FString& OutOldspeak, FString& OutExpectedNewspeak) const;

	/** Get the definition of a Newspeak word for the educational debrief */
	UFUNCTION(BlueprintCallable, Category = "1984|Newspeak")
	FString GetDefinition(const FString& NewspeakWord) const;

	/** Full structured dictionary (Oldspeak key → FNewspeakEntry) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Newspeak")
	TArray<FNewspeakEntry> Dictionary;

protected:
	/** Simple Oldspeak→Newspeak lookup (populated from Dictionary) */
	TMap<FString, FString> OldspeakToNewspeak;

	/** Newspeak→Oldspeak reverse lookup */
	TMap<FString, FString> NewspeakToOldspeak;

	/** Newspeak→Definition lookup */
	TMap<FString, FString> NewspeakDefinitions;

	/** Load the Newspeak dictionary */
	void LoadDictionary();

	/** Register one entry into all lookup maps */
	void AddEntry(const FString& Oldspeak, const FString& Newspeak,
	              const FString& Definition, ENewspeakCategory Category);

	/** Compute normalised Levenshtein edit distance in [0, 1] */
	static float EditDistanceScore(const FString& A, const FString& B);

	/** Apply intensifier prefix rules (very/extremely → plus/doubleplus) */
	static FString ApplyIntensifierRules(const FString& Word);
};
