// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/PlayerController.h"
#include "FPSCharacterController.generated.h"

class ASWeaponBase; 



UCLASS()
class ISOLATION_API ASCharacterController : public APlayerController
{
	GENERATED_BODY()

	public:

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int AmmoBoxCount;
};
