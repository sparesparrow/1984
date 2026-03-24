#include "NewSpeakProcessor.h"
#include "SurveillanceSystem.h"
#include "Kismet/GameplayStatics.h"

UNewSpeakProcessor::UNewSpeakProcessor()
{
}

void UNewSpeakProcessor::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadDictionary();

	UE_LOG(LogTemp, Log, TEXT("NewSpeakProcessor: Dictionary loaded with %d entries."), Dictionary.Num());
}

// ---------------------------------------------------------------------------
// Dictionary construction
// ---------------------------------------------------------------------------

void UNewSpeakProcessor::AddEntry(const FString& Oldspeak, const FString& Newspeak,
                                   const FString& Definition, ENewspeakCategory Category)
{
	FNewspeakEntry Entry;
	Entry.Oldspeak   = Oldspeak.ToLower();
	Entry.Newspeak   = Newspeak.ToLower();
	Entry.Definition = Definition;
	Entry.Category   = Category;
	Dictionary.Add(Entry);

	OldspeakToNewspeak.Add(Entry.Oldspeak, Entry.Newspeak);
	NewspeakToOldspeak.Add(Entry.Newspeak, Entry.Oldspeak);
	NewspeakDefinitions.Add(Entry.Newspeak, Entry.Definition);
}

void UNewSpeakProcessor::LoadDictionary()
{
	using C = ENewspeakCategory;

	// -----------------------------------------------------------------------
	// A Vocabulary — everyday stripped-down words
	// -----------------------------------------------------------------------
	AddEntry(TEXT("good"),      TEXT("good"),           TEXT("The only positive evaluative word; all gradations replaced by prefixes"), C::AVocabulary);
	AddEntry(TEXT("bad"),       TEXT("ungood"),          TEXT("Negation via un- prefix eliminates the concept of 'bad'"),               C::AVocabulary);
	AddEntry(TEXT("very good"), TEXT("plusgood"),        TEXT("Intensification via plus- prefix"),                                       C::AVocabulary);
	AddEntry(TEXT("excellent"), TEXT("doubleplusgood"),  TEXT("Maximum intensification"),                                                C::AVocabulary);
	AddEntry(TEXT("very bad"),  TEXT("plusungood"),      TEXT("Combined intensifier and negation"),                                      C::AVocabulary);
	AddEntry(TEXT("terrible"),  TEXT("doubleplusungood"),TEXT("Maximum negative intensification"),                                       C::AVocabulary);
	AddEntry(TEXT("free"),      TEXT("free"),            TEXT("Only in physical sense — 'free from lice'; politically abolished"),       C::AVocabulary);
	AddEntry(TEXT("dark"),      TEXT("unlight"),         TEXT("All antonyms replaced by un- prefix"),                                    C::AVocabulary);
	AddEntry(TEXT("strong"),    TEXT("strong"),          TEXT("Physical descriptor; political strength is doubleplusgood"),              C::AVocabulary);
	AddEntry(TEXT("weak"),      TEXT("unstrong"),        TEXT("Antonym via prefix"),                                                     C::AVocabulary);
	AddEntry(TEXT("fast"),      TEXT("fast"),            TEXT("Physical descriptor retained"),                                           C::AVocabulary);
	AddEntry(TEXT("slow"),      TEXT("unfast"),          TEXT("Antonym via prefix"),                                                     C::AVocabulary);

	// -----------------------------------------------------------------------
	// B Vocabulary — political compound words
	// -----------------------------------------------------------------------
	AddEntry(TEXT("orthodoxy"),              TEXT("goodthink"),    TEXT("Thinking in a manner acceptable to the Party"),                  C::BVocabulary);
	AddEntry(TEXT("thoughtcrime"),           TEXT("crimethink"),   TEXT("A thought contrary to Party doctrine; the gravest offense"),     C::BVocabulary);
	AddEntry(TEXT("protective stupidity"),   TEXT("crimestop"),    TEXT("Instinctive stopping before a dangerous thought"),               C::BVocabulary);
	AddEntry(TEXT("reality control"),        TEXT("doublethink"),  TEXT("Holding two contradictory beliefs simultaneously"),             C::BVocabulary);
	AddEntry(TEXT("loyal contradiction"),    TEXT("blackwhite"),   TEXT("Claiming black is white when the Party demands it"),             C::BVocabulary);
	AddEntry(TEXT("individualism"),          TEXT("ownlife"),      TEXT("Living for oneself; considered antisocial"),                     C::BVocabulary);
	AddEntry(TEXT("ministry of truth"),      TEXT("minitrue"),     TEXT("Ministry that rewrites history to match current Party line"),    C::BVocabulary);
	AddEntry(TEXT("ministry of peace"),      TEXT("minipax"),      TEXT("Ministry responsible for waging war"),                          C::BVocabulary);
	AddEntry(TEXT("ministry of love"),       TEXT("miniluv"),      TEXT("Ministry responsible for punishment and re-education"),          C::BVocabulary);
	AddEntry(TEXT("ministry of plenty"),     TEXT("miniplenty"),   TEXT("Ministry that administers rationing and economic scarcity"),     C::BVocabulary);
	AddEntry(TEXT("big brother"),            TEXT("bb"),           TEXT("The face of the Party; may or may not exist"),                  C::BVocabulary);
	AddEntry(TEXT("thought police"),         TEXT("thinkpol"),     TEXT("Secret police force that detects thoughtcrime"),                C::BVocabulary);
	AddEntry(TEXT("telescreen"),             TEXT("telescreen"),   TEXT("Two-way television surveillance device in every room"),          C::BVocabulary);
	AddEntry(TEXT("prole"),                  TEXT("prole"),        TEXT("Member of the proletariat; 85% of Oceania's population"),       C::BVocabulary);
	AddEntry(TEXT("oldspeak"),               TEXT("oldspeak"),     TEXT("Standard English; being phased out in favour of Newspeak"),     C::BVocabulary);
	AddEntry(TEXT("newspeak"),               TEXT("newspeak"),     TEXT("The official language of Oceania; designed to limit thought"),  C::BVocabulary);
	AddEntry(TEXT("speakwrite"),             TEXT("speakwrite"),   TEXT("Device for dictating text directly to written form"),           C::BVocabulary);
	AddEntry(TEXT("memory hole"),            TEXT("memhole"),      TEXT("Slot into which documents to be destroyed are inserted"),       C::BVocabulary);
	AddEntry(TEXT("unperson"),               TEXT("unperson"),     TEXT("Someone erased from all records as if they never existed"),     C::BVocabulary);
	AddEntry(TEXT("duckspeak"),              TEXT("duckspeak"),    TEXT("Uttering orthodox sounds with no thought; a compliment"),       C::BVocabulary);
	AddEntry(TEXT("bellyfeel"),              TEXT("bellyfeel"),    TEXT("Blind enthusiastic acceptance without understanding"),          C::BVocabulary);

	// -----------------------------------------------------------------------
	// C Vocabulary — scientific/technical (mostly abolished)
	// -----------------------------------------------------------------------
	AddEntry(TEXT("science"),    TEXT("ingsoc"),         TEXT("Scientific knowledge replaced by Party doctrine"),                        C::CVocabulary);
	AddEntry(TEXT("freedom"),    TEXT("crimethink"),     TEXT("The concept of freedom is thoughtcrime"),                               C::CVocabulary);
	AddEntry(TEXT("justice"),    TEXT("crimethink"),     TEXT("Concepts of justice outside Party law are thoughtcrime"),               C::CVocabulary);
	AddEntry(TEXT("democracy"),  TEXT("crimethink"),     TEXT("Democratic concepts are doublepluscrimethink"),                         C::CVocabulary);
	AddEntry(TEXT("equality"),   TEXT("crimethink"),     TEXT("The notion of equality outside Party hierarchy is thoughtcrime"),       C::CVocabulary);
	AddEntry(TEXT("love"),       TEXT("sexcrime"),       TEXT("Romantic love outside approved procreation is a sexcrime"),             C::CVocabulary);
	AddEntry(TEXT("history"),    TEXT("rectified"),      TEXT("History is continuously 'rectified' to match current Party truth"),     C::CVocabulary);
	AddEntry(TEXT("truth"),      TEXT("truth"),          TEXT("There is only one truth: what the Party says is true"),                 C::CVocabulary);
	AddEntry(TEXT("war"),        TEXT("peace"),          TEXT("WAR IS PEACE — the Party slogan"),                                     C::CVocabulary);
	AddEntry(TEXT("slavery"),    TEXT("freedom"),        TEXT("FREEDOM IS SLAVERY — the Party slogan"),                               C::CVocabulary);
	AddEntry(TEXT("ignorance"),  TEXT("strength"),       TEXT("IGNORANCE IS STRENGTH — the Party slogan"),                            C::CVocabulary);
}

// ---------------------------------------------------------------------------
// Core API
// ---------------------------------------------------------------------------

bool UNewSpeakProcessor::IsValidNewspeak(const FString& Word) const
{
	return NewspeakToOldspeak.Contains(Word.ToLower())
		|| OldspeakToNewspeak.Contains(Word.ToLower());
}

FString UNewSpeakProcessor::TranslateToNewspeak(const FString& OldspeakPhrase) const
{
	// Tokenise the phrase into words
	TArray<FString> Words;
	OldspeakPhrase.ParseIntoArrayWS(Words);

	TArray<FString> Result;
	int32 i = 0;
	while (i < Words.Num())
	{
		// Try bi-gram (two consecutive words) first
		if (i + 1 < Words.Num())
		{
			const FString BiGram = (Words[i] + TEXT(" ") + Words[i + 1]).ToLower();
			if (const FString* NS = OldspeakToNewspeak.Find(BiGram))
			{
				Result.Add(*NS);
				i += 2;
				continue;
			}
		}

		// Apply intensifier prefix rules (very, extremely)
		const FString Lower = Words[i].ToLower();
		if (Lower == TEXT("very") || Lower == TEXT("extremely") || Lower == TEXT("absolutely"))
		{
			// Peek ahead and wrap next word with plus-/doubleplus- prefix
			if (i + 1 < Words.Num())
			{
				const FString NextWord = Words[i + 1].ToLower();
				const FString* NS = OldspeakToNewspeak.Find(NextWord);
				const FString Base = NS ? *NS : NextWord;
				const FString Prefix = (Lower == TEXT("extremely") || Lower == TEXT("absolutely"))
					? TEXT("doubleplus") : TEXT("plus");
				Result.Add(Prefix + Base);
				i += 2;
				continue;
			}
		}

		// Single-word lookup
		if (const FString* NS = OldspeakToNewspeak.Find(Lower))
		{
			Result.Add(*NS);
		}
		else
		{
			// Word has no Newspeak equivalent — keep as-is (it is A-vocabulary)
			Result.Add(Words[i]);
		}

		++i;
	}

	return FString::Join(Result, TEXT(" "));
}

FString UNewSpeakProcessor::TranslateToOldspeak(const FString& NewspeakPhrase) const
{
	TArray<FString> Words;
	NewspeakPhrase.ParseIntoArrayWS(Words);

	TArray<FString> Result;
	for (const FString& Word : Words)
	{
		const FString Lower = Word.ToLower();

		// Strip known prefixes for reverse lookup
		FString Stem = Lower;
		FString Prefix;
		if (Lower.StartsWith(TEXT("doubleplus")))      { Stem = Lower.Mid(10); Prefix = TEXT("extremely "); }
		else if (Lower.StartsWith(TEXT("plus")))        { Stem = Lower.Mid(4);  Prefix = TEXT("very "); }
		else if (Lower.StartsWith(TEXT("un")))          { Stem = Lower.Mid(2);  Prefix = TEXT("not "); }

		if (const FString* OS = NewspeakToOldspeak.Find(Stem))
		{
			Result.Add(Prefix + *OS);
		}
		else if (const FString* OS2 = NewspeakToOldspeak.Find(Lower))
		{
			Result.Add(*OS2);
		}
		else
		{
			Result.Add(Word);
		}
	}

	return FString::Join(Result, TEXT(" "));
}

float UNewSpeakProcessor::ScoreNewspeakAttempt(const FString& Expected, const FString& PlayerInput) const
{
	// Exact match (case-insensitive)
	if (Expected.Equals(PlayerInput, ESearchCase::IgnoreCase))
	{
		return 1.0f;
	}

	// Fuzzy match via normalised edit distance
	const float EditScore = EditDistanceScore(Expected.ToLower(), PlayerInput.ToLower());

	// Generous threshold: >= 0.75 similarity counts as correct for voice recognition imprecision
	return EditScore;
}

void UNewSpeakProcessor::ProcessVoiceInput(const FString& RecognizedText)
{
	// Meta Voice SDK integration:
	// The recognized text arrives from VoiceSDK OnPartialTranscription / OnFullTranscription delegates.
	// Blueprint-side wires the delegate; C++ validates the content.

	const FString Lower = RecognizedText.ToLower();
	UE_LOG(LogTemp, Log, TEXT("NewSpeakProcessor: Voice input received: '%s'"), *RecognizedText);

	bool bIsValidNewspeak = IsValidNewspeak(Lower);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USurveillanceSystem* SS = GI->GetSubsystem<USurveillanceSystem>())
		{
			if (bIsValidNewspeak)
			{
				// Speaking Newspeak correctly reduces suspicion (loyal behaviour)
				SS->ReportIncident(
					ESuspicionEvent::SpeakingNewspeak,
					USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::SpeakingNewspeak));
				UE_LOG(LogTemp, Log, TEXT("NewSpeakProcessor: Correct Newspeak — suspicion reduced."));
			}
			else
			{
				// Speaking Oldspeak in public raises suspicion
				SS->ReportIncident(
					ESuspicionEvent::SpeakingOldspeak,
					USurveillanceSystem::GetDefaultEventWeight(ESuspicionEvent::SpeakingOldspeak));
				UE_LOG(LogTemp, Warning, TEXT("NewSpeakProcessor: Oldspeak detected — suspicion raised."));
			}
		}
	}
}

void UNewSpeakProcessor::GetTranslationChallenge(int32 ActIndex, FString& OutOldspeak, FString& OutExpectedNewspeak) const
{
	if (Dictionary.Num() == 0)
	{
		OutOldspeak          = TEXT("freedom");
		OutExpectedNewspeak  = TEXT("crimethink");
		return;
	}

	// Scale difficulty by act: early acts use simple A-vocabulary,
	// later acts introduce B-vocabulary political compound words.
	// Act 0-1 → AVocabulary, Act 2-3 → BVocabulary, Act 4 → all
	TArray<FNewspeakEntry> Candidates;
	for (const FNewspeakEntry& Entry : Dictionary)
	{
		bool bInclude = false;
		if (ActIndex <= 1) bInclude = (Entry.Category == ENewspeakCategory::AVocabulary);
		else if (ActIndex <= 3) bInclude = (Entry.Category != ENewspeakCategory::CVocabulary);
		else bInclude = true;

		if (bInclude)
		{
			Candidates.Add(Entry);
		}
	}

	if (Candidates.Num() == 0)
	{
		Candidates = Dictionary;
	}

	// Pseudo-random selection using world time as seed for variety
	const int32 Idx = FMath::RandRange(0, Candidates.Num() - 1);
	OutOldspeak         = Candidates[Idx].Oldspeak;
	OutExpectedNewspeak = Candidates[Idx].Newspeak;
}

FString UNewSpeakProcessor::GetDefinition(const FString& NewspeakWord) const
{
	if (const FString* Def = NewspeakDefinitions.Find(NewspeakWord.ToLower()))
	{
		return *Def;
	}
	return FString::Printf(TEXT("No definition found for '%s'."), *NewspeakWord);
}

// ---------------------------------------------------------------------------
// Levenshtein edit distance (normalised to [0,1])
// ---------------------------------------------------------------------------

float UNewSpeakProcessor::EditDistanceScore(const FString& A, const FString& B)
{
	const int32 LenA = A.Len();
	const int32 LenB = B.Len();
	if (LenA == 0) return LenB == 0 ? 1.0f : 0.0f;
	if (LenB == 0) return 0.0f;

	// DP matrix (row-compressed)
	TArray<int32> Prev, Curr;
	Prev.SetNumZeroed(LenB + 1);
	Curr.SetNumZeroed(LenB + 1);

	for (int32 j = 0; j <= LenB; ++j) Prev[j] = j;

	for (int32 i = 1; i <= LenA; ++i)
	{
		Curr[0] = i;
		for (int32 j = 1; j <= LenB; ++j)
		{
			const int32 Cost = (A[i - 1] == B[j - 1]) ? 0 : 1;
			Curr[j] = FMath::Min3(
				Prev[j]     + 1,
				Curr[j - 1] + 1,
				Prev[j - 1] + Cost);
		}
		Swap(Prev, Curr);
	}

	const int32 EditDist = Prev[LenB];
	const int32 MaxLen   = FMath::Max(LenA, LenB);
	return 1.0f - static_cast<float>(EditDist) / static_cast<float>(MaxLen);
}

FString UNewSpeakProcessor::ApplyIntensifierRules(const FString& Word)
{
	const FString Lower = Word.ToLower();
	if (Lower.StartsWith(TEXT("very ")))
	{
		return TEXT("plus") + Lower.Mid(5);
	}
	if (Lower.StartsWith(TEXT("extremely ")) || Lower.StartsWith(TEXT("absolutely ")))
	{
		const int32 SpaceIdx = Lower.Find(TEXT(" "));
		return TEXT("doubleplus") + Lower.Mid(SpaceIdx + 1);
	}
	return Word;
}
