// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIManager.h"

void UAIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Subsystem is running"));
}
