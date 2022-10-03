// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AIManager.generated.h"

/**
 * 
 */

USTRUCT()
struct FGlobalCombatParameters
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	int NumEngagers;
	
	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	int NumAmbushers;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	int NumSearchers;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float EngagerReplacementDelay;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	int MinShooters;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	int MaxShooters;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float SnuckAwayDistance;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	bool AllowPursueOnTimeout;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float WaitToAdvanceTime;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float MoveAdvanceDistanceReset;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float WaitToFlankTime;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float MinFlankPathRating;

	UPROPERTY(EditInstanceOnly, Category = "Global Combat Parameters")
	float MoveFlankDistanceReset;
};

UCLASS()
class ISOLATION_API UAIManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:

	/**	Updates the global combat parameters */
	void UpdateGlobalCombatParameters(const FGlobalCombatParameters NewGlobalCombatParameters)
	{
		GlobalCombatParameters = NewGlobalCombatParameters;

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::SanitizeFloat(GlobalCombatParameters.MinShooters));
	}

private:

	FGlobalCombatParameters GlobalCombatParameters;
};
