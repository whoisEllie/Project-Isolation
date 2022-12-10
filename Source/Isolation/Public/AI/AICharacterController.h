// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICharacterController.generated.h"

UENUM()
enum class Attitude : uint8
{
	Aggressive,
	Defensive
};

UENUM()
enum class Speed : uint8
{
	Slow,
	Medium,
	Fast
};

UENUM()
enum class PreferredEngagementRange : uint8
{
	Close,
	Medium,
	Far
};
/**
 * 
 */
UCLASS()
class ISOLATION_API AAICharacterController : public AAIController
{
	GENERATED_BODY()

public:

	UPROPERTY()
	AActor* TargetActor;

private:

	float CombatMinDistance;

	float CombatMaxDistance;

	Attitude Attitude;

	Speed Speed;

	PreferredEngagementRange PreferredEngagementRange;

	bool bAllowCover;

	bool bAllowAmbush;

	bool bAllowHardpointAssignment;
};
