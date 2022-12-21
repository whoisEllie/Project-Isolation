// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISense.h"
#include "AICharacterController.generated.h"

class UAISenseConfig_Sight;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerceptionUpdateHandlingDelegate, const TArray<AActor*>&, UpdatedActors);

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
	AAICharacterController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly)
	UAIPerceptionComponent* AiPerceptionComponent;

	// Overriding team
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FPerceptionUpdateHandlingDelegate PerceptionUpdateHandlingDelegate;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	AActor* GetTargetActor() const { return TargetActor; }

private:

	float CombatMinDistance;

	float CombatMaxDistance;

	Attitude Attitude;

	Speed Speed;

	PreferredEngagementRange PreferredEngagementRange;

	bool bAllowCover;

	bool bAllowAmbush;

	bool bAllowHardpointAssignment;
	
	UPROPERTY()
	AActor* TargetActor;

	int Partition(TArray<AActor*> *InArray, int Start, int End) const;

	void QuickSort(TArray<AActor*> *InArray, int Start, int End);

	UFUNCTION()
	void HandlePerceptionUpdate(const TArray<AActor*>& UpdatedActors);

	UFUNCTION(BlueprintCallable)
	void UpdateTargetActor();

	UPROPERTY()
	TArray<AActor*> TargetsArray;
};
