#include "PropagandaHUD.h"

APropagandaHUD::APropagandaHUD()
{
}

void APropagandaHUD::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Find all world-space widget components in the level
	// TODO: Initialize propaganda content rotation
	RefreshPropagandaContent();
}

void APropagandaHUD::DisplayPartySlogan(const FString& Slogan)
{
	// TODO: Update world-space text widgets with the slogan
	// Core slogans: "WAR IS PEACE", "FREEDOM IS SLAVERY", "IGNORANCE IS STRENGTH"
	// TODO: Animate slogan appearance (typewriter effect on telescreen)
}

void APropagandaHUD::UpdateSuspicionIndicator(float SuspicionLevel)
{
	// TODO: Diegetic suspicion feedback:
	// 0.0-0.3: Normal — Big Brother posters neutral
	// 0.3-0.6: Eyes on posters track player more aggressively
	// 0.6-0.8: Red tint on telescreen edges, ambient audio tension
	// 0.8-1.0: Direct warnings from telescreens, flashing alerts
}

void APropagandaHUD::ShowNewspeakChallenge(const FString& OldspeakPhrase, const FString& ExpectedNewspeak)
{
	// TODO: Display on Winston's work terminal (Ministry of Truth desk)
	// TODO: Show Oldspeak text with prompt to translate
	// TODO: Enable input for written or spoken response
}

void APropagandaHUD::HideNewspeakChallenge()
{
	// TODO: Clear the Newspeak challenge display
}

void APropagandaHUD::ShowDebriefScreen(const FString& EndingType, const FString& DecisionLogJSON)
{
	// TODO: Break fourth wall — show educational debrief
	// TODO: Display ending type and what it means
	// TODO: Show decision log summary
	// TODO: Present real-world connections to totalitarian history
	// TODO: Display reflection prompts for classroom discussion
}

void APropagandaHUD::RefreshPropagandaContent()
{
	// TODO: Rotate Party propaganda content on all world-space displays
	// TODO: Content varies by narrative act:
	// Act I: Standard Party announcements, Big Brother imagery
	// Act II: Increased war rhetoric, enemy nation propaganda
	// Act III: Crackdown announcements, loyalty oaths
	// Act IV: Interrogation-room specific content
}
