// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ISOLATION_API SDebugHelpers
{
public:
	SDebugHelpers();
	~SDebugHelpers();

	static void PrintStrArray(TArray<FString> Arr, FColor Color, float DisplayTime);

	static void PrintNameArray(TArray<FName> Arr, FColor Color, float DisplayTime);

	static void PrintIntArray(TArray<int> Arr, FColor Color, float DisplayTime);

	static void PrintFloatArray(TArray<float> Arr, FColor Color, float DisplayTime);
};
