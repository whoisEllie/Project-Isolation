// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SWeaponStatsCalculator.generated.h"

/**
 * 
 */
UCLASS()
class ISOLATION_API USWeaponStatsCalculator : public UObject
{
	GENERATED_BODY()

public:
	
	// Returns a value between 0 and 1, representing the overall accuracy rating of the weapon
	static float GetAccuracyRating(float PitchVariation, float PitchModifier, float YawVariation, float YawModifier);

	static float GetRateOfFireRating(int RateOfFire);

	static float GetDamageRating(float BaseDamage, float HeadshotMultiplier);

	static float GetStabilityRating(float RecoilCurvePeak, float RecoilModifier);

	static float GetWeaponRating(float PitchVariation, float PitchModifier, float YawVariation, float YawModifier,
		int RateOfFire, float BaseDamage, float HeadshotMultiplier, float RecoilCurvePeak, float RecoilModifier);
};
