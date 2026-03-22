#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WinstonCharacter.generated.h"

class USuspicionComponent;

/**
 * AWinstonCharacter
 *
 * The player character — Winston Smith. In VR, the player IS Winston.
 * This character manages the dual-life mechanic: outer conformity vs.
 * inner resistance. The SuspicionComponent tracks how well the player
 * maintains their facade.
 */
UCLASS()
class PROJECT1984_API AWinstonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWinstonCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** The player's personal suspicion tracking */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Surveillance")
	USuspicionComponent* SuspicionComponent;

	/** Whether the player is currently writing in the diary */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|State")
	bool bIsWritingDiary;

	/** Whether the player possesses the forbidden diary */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1984|Inventory")
	bool bHasDiary;

	/** Begin diary writing interaction */
	UFUNCTION(BlueprintCallable, Category = "1984|Interaction")
	void StartDiaryWriting();

	/** End diary writing interaction */
	UFUNCTION(BlueprintCallable, Category = "1984|Interaction")
	void StopDiaryWriting();

	/** Check if the player is in a surveilled area */
	UFUNCTION(BlueprintCallable, Category = "1984|Surveillance")
	bool IsBeingWatched() const;

protected:
	/** Check for nearby telescreens and surveillance actors */
	void UpdateSurveillanceStatus();
};
