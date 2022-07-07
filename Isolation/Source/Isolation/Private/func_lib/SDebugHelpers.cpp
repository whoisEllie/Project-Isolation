// Fill out your copyright notice in the Description page of Project Settings.


#include "func_lib/SDebugHelpers.h"

SDebugHelpers::SDebugHelpers()
{
}

SDebugHelpers::~SDebugHelpers()
{
}

void SDebugHelpers::PrintStrArray(TArray<FString> Arr, FColor Color, float DisplayTime)
{
	for (FString element: Arr)
	{
		GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color, element);
	}
}

void SDebugHelpers::PrintNameArray(TArray<FName> Arr, FColor Color, float DisplayTime)
{
	for (FName element: Arr)
	{
		GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color, element.ToString());
	}
}

void SDebugHelpers::PrintIntArray(TArray<int> Arr, FColor Color, float DisplayTime)
{
	for (int element: Arr)
	{
		GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color, FString::FromInt(element));
	}
}

void SDebugHelpers::PrintFloatArray(TArray<float> Arr, FColor Color, float DisplayTime)
{
	for (float element: Arr)
	{
		GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color, FString::SanitizeFloat(element));
	}
}
