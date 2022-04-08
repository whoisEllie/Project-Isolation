// Fill out your copyright notice in the Description page of Project Settings.


#include "func_lib/SWeaponStatsCalculator.h"
#include "Math/UnrealMathUtility.h"

float USWeaponStatsCalculator::GetAccuracyRating(const float PitchVariation, const float PitchModifier, const float YawVariation,
	const float YawModifier)
{
	const float MaxAccuracy = (PitchVariation + PitchModifier) + (YawVariation + YawModifier);

	return FMath::Clamp((4.0f-MaxAccuracy)/4.0f, 0.0f, 1.0f);
}

float USWeaponStatsCalculator::GetRateOfFireRating(const int RateOfFire)
{
	const float RoF = RateOfFire;
	
	return FMath::Clamp((1000.0f-RoF)/1000.0f, 0.0f, 1.0f);
}

float USWeaponStatsCalculator::GetDamageRating(const float BaseDamage, const float HeadshotMultiplier)
{
	const float HeadshotDamage = BaseDamage * HeadshotMultiplier;

	return FMath::Clamp((100.0f-HeadshotDamage)/100.0f, 0.0f, 1.0f);
}

float USWeaponStatsCalculator::GetStabilityRating(const float RecoilCurvePeak, const float RecoilModifier)
{
	const float MaxRecoil = RecoilCurvePeak*RecoilModifier;

	return FMath::Clamp((1.5f-MaxRecoil)/1.5f, 0.0f, 1.0f);
}

float USWeaponStatsCalculator::GetWeaponRating(const float PitchVariation, const float PitchModifier, const float YawVariation,
	const float YawModifier, const int RateOfFire, const float BaseDamage, const float HeadshotMultiplier, const float RecoilCurvePeak,
	const float RecoilModifier)
{
	const float AccuracyRating = GetAccuracyRating(PitchVariation, PitchModifier, YawVariation, YawModifier);
	const float RateOfFireRating = GetRateOfFireRating(RateOfFire);
	const float DamageRating = GetDamageRating(BaseDamage, HeadshotMultiplier);
	const float StabilityRating = GetStabilityRating(RecoilCurvePeak, RecoilModifier);

	return FMath::Clamp((AccuracyRating + RateOfFireRating + DamageRating + StabilityRating)/4.0f, 0.0f, 1.0f);
	
}







