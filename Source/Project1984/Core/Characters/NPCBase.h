#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCBase.generated.h"

/**
 * NPC loyalty classification — determines reporting behavior
 */
UENUM(BlueprintType)
enum class ENPCLoyalty : uint8
{
	/** Devout Party member — always reports suspicious behavior */
	PartyFaithful		UMETA(DisplayName = "Party Faithful"),
	/** Outer Party member — reports if behavior is overt */
	OuterParty			UMETA(DisplayName = "Outer Party"),
	/** Prole — generally ignores Party rules */
	Prole				UMETA(DisplayName = "Prole"),
	/** Thought Police — undercover agent, always watching */
	ThoughtPolice		UMETA(DisplayName = "Thought Police")
};

/**
 * ANPCBase
 *
 * Base class for all NPCs in Airstrip One. NPCs observe the player,
 * follow patrol routes, and can report suspicious behavior to the
 * SurveillanceSystem based on their loyalty classification.
 */
UCLASS()
class PROJECT1984_API ANPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** NPC's loyalty to the Party — affects reporting behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|NPC")
	ENPCLoyalty Loyalty;

	/** Detection radius for observing player behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1984|NPC")
	float DetectionRadius;

	/** Whether this NPC is currently suspicious of the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|NPC")
	bool bIsSuspicious;

	/** Report player behavior to the SurveillanceSystem */
	UFUNCTION(BlueprintCallable, Category = "1984|NPC")
	void ReportPlayer();

	/** Check if the NPC can see the player */
	UFUNCTION(BlueprintCallable, Category = "1984|NPC")
	bool CanSeePlayer() const;

protected:
	/** Run patrol behavior */
	virtual void ExecutePatrol();

	/** Evaluate whether observed player behavior is suspicious */
	virtual void EvaluatePlayerBehavior();
};
