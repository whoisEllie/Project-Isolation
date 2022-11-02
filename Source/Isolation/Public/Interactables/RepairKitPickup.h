// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionActor.h"
#include "RepairKitPickup.generated.h"

/**
 * 
 */
UCLASS()
class ISOLATION_API ARepairKitPickup : public AInteractionActor
{
	GENERATED_BODY()
	
	/** Called from the interact interface */
	virtual void Interact(AActor* InteractionDelegate) override;

	virtual void BeginPlay() override;

	/** The sound effect to play when the pickup is collected */
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases	")
	USoundBase* PickupSFX;
};
