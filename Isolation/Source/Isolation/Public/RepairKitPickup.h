// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionActor.h"
#include "RepairKitPickup.generated.h"

/**
 * 
 */
UCLASS()
class ISOLATION_API ASRepairKitPickup : public ASInteractionActor
{
	GENERATED_BODY()

	virtual void Interact() override;

	virtual void BeginPlay() override;

	// The sound effect to play when the pickup is collected
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases	")
	USoundBase* PickupSFX;
};
