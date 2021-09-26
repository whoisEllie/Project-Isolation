// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SIsolationGameInstance.generated.h"

// Struct holding the information for spawning a new enemy AI (spawn Location, target locations, weapon)
USTRUCT(BlueprintType)
struct FsingleWeaponParams
{
    GENERATED_BODY()
    	
    // Ammunition
    
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        int clipCapacity; // The maximum size of the player's magazine
    	    
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        int clipSize; // The amount of ammunition currently in the magazine
    
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        FString ammoType; // Choose between 'pistol' 'rifle' 'shotgun' or 'special'
    	    
    // Weapon Health
    	
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
    float weaponHealth; // The current health of the weapon (degredation values are in the weapon class)
};

UCLASS()
class ISOLATION_API USIsolationGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        TArray<FsingleWeaponParams> weaponParameters;

    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        int pistolAmmo;
        
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        int rifleAmmo;
    
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        int shotgunAmmo;
    
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
        int specialAmmo;
};
