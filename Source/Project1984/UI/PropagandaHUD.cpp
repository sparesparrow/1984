#include "PropagandaHUD.h"
#include "Project1984/Core/Systems/SurveillanceSystem.h"
#include "Project1984/Core/Systems/NarrativeManager.h"
#include "Project1984/VR/TelescreenComponent.h"
#include "Kismet/GameplayStatics.h"

// Act-aware propaganda content sets
// Each act expands the set to reflect escalating Party pressure.
namespace PropagandaContent
{
	static const TCHAR* Slogans[] = {
		TEXT("WAR IS PEACE"),                                         // 0
		TEXT("FREEDOM IS SLAVERY"),                                   // 1
		TEXT("IGNORANCE IS STRENGTH"),                                // 2
		TEXT("BIG BROTHER IS WATCHING YOU"),                          // 3
		TEXT("2 + 2 = 5"),                                            // 4
		TEXT("OCEANIA HAS ALWAYS BEEN AT WAR WITH EASTASIA"),         // 5
	};

	// Per-act slogan pool sizes (cumulative — later acts include all prior slogans)
	static constexpr int32 ActSloganCounts[] = { 3, 4, 5, 6, 6 };

	static int32 GetPoolSize(ENarrativeAct Act)
	{
		const int32 Idx = FMath::Clamp(static_cast<int32>(Act), 0, 4);
		return ActSloganCounts[Idx];
	}

	// Educational debrief endings
	struct FEndingInfo
	{
		const TCHAR* Type;
		const TCHAR* Meaning;
		const TCHAR* RealWorldParallel;
	};

	static const FEndingInfo Endings[] = {
		{
			TEXT("TotalCompliance"),
			TEXT("Winston loved Big Brother. Every act of resistance was erased; the conditioning succeeded completely."),
			TEXT("Mirrors the psychological process of 'learned helplessness' documented in authoritarian regimes.")
		},
		{
			TEXT("PartialResistance"),
			TEXT("Winston attempted resistance but ultimately broke under Room 101 conditioning. The Party won — but he made them work for it."),
			TEXT("Reflects the testimony of many dissidents under totalitarian pressure: resistance is possible but rarely permanent without external support.")
		},
		{
			TEXT("Martyrdom"),
			TEXT("Winston maintained his inner resistance to the end. The Party cannot erase what he knew — only what he said."),
			TEXT("Echoes figures like Solzhenitsyn: the unbroken mind as the final form of dissent.")
		},
	};
}

APropagandaHUD::APropagandaHUD()
{
	SurveillanceSystem       = nullptr;
	NarrativeManager         = nullptr;
	SloganIndex              = 0;
	bNewspeakChallengeActive = false;
}

void APropagandaHUD::BeginPlay()
{
	Super::BeginPlay();

	// Resolve subsystems
	if (UGameInstance* GI = GetGameInstance())
	{
		SurveillanceSystem = GI->GetSubsystem<USurveillanceSystem>();
		NarrativeManager   = GI->GetSubsystem<UNarrativeManager>();
	}

	// Bind to suspicion threshold changes so we can update diegetic indicators
	if (SurveillanceSystem)
	{
		SurveillanceSystem->OnSuspicionThresholdChanged.AddDynamic(
			this, &APropagandaHUD::OnSuspicionThresholdChanged);
	}

	// Start automatic slogan rotation every 20 seconds
	GetWorldTimerManager().SetTimer(
		SloganRotationTimer, this, &APropagandaHUD::RotateSloganTick,
		20.0f, /*bLoop=*/true, /*FirstDelay=*/5.0f);

	// Show initial propaganda for the current act
	RefreshPropagandaContent();

	UE_LOG(LogTemp, Log, TEXT("PropagandaHUD: Initialized. Slogan rotation active (20s interval)."));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void APropagandaHUD::DisplayPartySlogan(const FString& Slogan)
{
	ActiveSlogan = Slogan;
	OnSloganChanged.Broadcast(Slogan);

	UE_LOG(LogTemp, Log, TEXT("PropagandaHUD: Broadcasting — \"%s\""), *Slogan);
	// Blueprint-side: Party poster UTextRenderComponents / UWidgetComponents on
	// poster actors bind to OnSloganChanged and update their text + typewriter anim.
}

void APropagandaHUD::UpdateSuspicionIndicator(float SuspicionLevel)
{
	OnSuspicionStateChanged.Broadcast(SuspicionLevel);

	if (SuspicionLevel < 0.3f)
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("PropagandaHUD: Suspicion %.2f — posters neutral, routine surveillance."), SuspicionLevel);
	}
	else if (SuspicionLevel < 0.6f)
	{
		UE_LOG(LogTemp, Log,
			TEXT("PropagandaHUD: Suspicion %.2f — poster eyes TRACKING player aggressively."), SuspicionLevel);
		// Blueprint poster actors enter 'tracking' state via OnSuspicionStateChanged
	}
	else if (SuspicionLevel < 0.8f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("PropagandaHUD: Suspicion %.2f — TELESCREEN EDGE TINT active. Thought Police alert zone."), SuspicionLevel);
		// Blueprint: telescreen border post-process tints red at this threshold
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("PropagandaHUD: Suspicion %.2f — DIRECT TELESCREEN WARNING. Imminent capture."), SuspicionLevel);

		// Force all Broadcasting telescreens to switch to Addressing state (direct player confrontation)
		for (TObjectIterator<UTelescreenComponent> It; It; ++It)
		{
			if (It->GetWorld() == GetWorld() && It->State == ETelescreenState::Broadcasting)
			{
				It->State = ETelescreenState::Addressing;
			}
		}
	}
}

void APropagandaHUD::ShowNewspeakChallenge(const FString& OldspeakPhrase, const FString& ExpectedNewspeak)
{
	bNewspeakChallengeActive  = true;
	NewspeakChallengeOldspeak = OldspeakPhrase;
	NewspeakChallengeExpected = ExpectedNewspeak;

	UE_LOG(LogTemp, Log,
		TEXT("PropagandaHUD: Newspeak challenge — Translate \"%s\" (expected: \"%s\")"),
		*OldspeakPhrase, *ExpectedNewspeak);
	// Blueprint: work terminal UWidgetComponent reads bNewspeakChallengeActive and
	// NewspeakChallengeOldspeak to show the translation prompt. Input is handled
	// by the NewSpeakProcessor via voice or keyboard.
}

void APropagandaHUD::HideNewspeakChallenge()
{
	bNewspeakChallengeActive  = false;
	NewspeakChallengeOldspeak.Empty();
	NewspeakChallengeExpected.Empty();

	UE_LOG(LogTemp, Log, TEXT("PropagandaHUD: Newspeak challenge dismissed."));
}

void APropagandaHUD::ShowDebriefScreen(const FString& EndingType, const FString& DecisionLogJSON)
{
	// Build formatted educational debrief text
	FString DebriefText;

	// --- Ending meaning ---
	for (const auto& Ending : PropagandaContent::Endings)
	{
		if (EndingType.Equals(Ending.Type, ESearchCase::IgnoreCase))
		{
			DebriefText += FString::Printf(
				TEXT("ENDING: %s\n%s\n\nReal-world context: %s\n\n"),
				Ending.Type, Ending.Meaning, Ending.RealWorldParallel);
			break;
		}
	}

	// --- Decision summary from SurveillanceSystem ---
	if (SurveillanceSystem && SurveillanceSystem->DecisionHistory.Num() > 0)
	{
		DebriefText += FString::Printf(
			TEXT("YOUR DECISIONS (%d incidents recorded):\n"),
			SurveillanceSystem->DecisionHistory.Num());
		for (const FString& Entry : SurveillanceSystem->DecisionHistory)
		{
			DebriefText += TEXT("  ") + Entry + TEXT("\n");
		}
		DebriefText += TEXT("\n");
	}

	// --- Real-world connections ---
	DebriefText += TEXT("REAL-WORLD PARALLELS:\n");
	DebriefText += TEXT("  • Telescreens mirror Bentham's Panopticon — constant surveillance changes behaviour even without active watchers.\n");
	DebriefText += TEXT("  • Newspeak parallels real language reforms: Soviet 'Newspeak', Maoist slogans, and modern euphemism treadmills.\n");
	DebriefText += TEXT("  • Orwell based the Ministry of Truth on his work at the BBC's wartime Eastern Service.\n");
	DebriefText += TEXT("  • The two-minutes hate draws from documented mass-hysteria techniques used in 20th-century totalitarian states.\n\n");

	// --- Classroom reflection prompts ---
	DebriefText += TEXT("DISCUSSION QUESTIONS:\n");
	DebriefText += TEXT("  1. How does constant surveillance change individual behaviour — even when no one is actively watching?\n");
	DebriefText += TEXT("  2. What role does language play in shaping — and limiting — what we can think?\n");
	DebriefText += TEXT("  3. Where do you see elements of Newspeak or doublethink in contemporary media and political speech?\n");
	DebriefText += TEXT("  4. What conditions allow ordinary citizens to become enforcers of an oppressive system?\n");
	DebriefText += TEXT("  5. Is Winston's inner resistance meaningful even if it produces no external change?\n");

	// Broadcast for Blueprint debrief screen UI actor
	OnShowDebrief.Broadcast(EndingType, DebriefText);

	UE_LOG(LogTemp, Log, TEXT("PropagandaHUD: Educational debrief displayed.\n%s"), *DebriefText);
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void APropagandaHUD::RefreshPropagandaContent()
{
	const ENarrativeAct Act = NarrativeManager
		? NarrativeManager->CurrentAct
		: ENarrativeAct::OrdinaryLife;

	const int32 PoolSize = PropagandaContent::GetPoolSize(Act);

	// Clamp SloganIndex into the act's pool
	SloganIndex = SloganIndex % PoolSize;

	const FString Slogan = FString(PropagandaContent::Slogans[SloganIndex]);
	DisplayPartySlogan(Slogan);
}

void APropagandaHUD::RotateSloganTick()
{
	const ENarrativeAct Act = NarrativeManager
		? NarrativeManager->CurrentAct
		: ENarrativeAct::OrdinaryLife;

	const int32 PoolSize = PropagandaContent::GetPoolSize(Act);
	SloganIndex          = (SloganIndex + 1) % PoolSize;

	RefreshPropagandaContent();
}

void APropagandaHUD::OnSuspicionThresholdChanged(float NewLevel, float OldLevel)
{
	UpdateSuspicionIndicator(NewLevel);
}
