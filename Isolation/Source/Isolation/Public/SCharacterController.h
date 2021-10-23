// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SCharacterController.generated.h"

class ASWeaponBase; 

// Enumerator holding the 4 types of ammunition that weapons can use (used as part of the FsingleWeaponParams struct)
// and to keep track of the total ammo the player has (ammoMap)
UENUM(BlueprintType)
enum AmmoType
{
	VE_Pistol       UMETA(DisplayName = "Pistol Ammo"),
	VE_Rifle        UMETA(DisplayName = "Rifle Ammo"),
	VE_Shotgun      UMETA(DisplayName = "Shotgun Ammo"),
	VE_Special		UMETA(DisplayName = "Special Ammo"),	
};

// Struct holding the information for spawning a new enemy AI (spawn Location, target locations, weapon)
USTRUCT(BlueprintType)
struct FsingleWeaponParams
{
    GENERATED_BODY()
    	
    // Ammunition
    
    // The maximum size of the player's magazine
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
    int clipCapacity; 

    // The amount of ammunition currently in the magazine
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
    int clipSize;
    
    // Enumerator holding the 4 possible ammo types defined above
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Type")
    TEnumAsByte<AmmoType> ammoType;
    	    
    // Weapon Health
    	
    // The current health of the weapon (degredation values are in the weapon class)
    UPROPERTY(EditDefaultsOnly, Category = "Variables")
    float weaponHealth;
};

UCLASS()
class ISOLATION_API ASCharacterController : public APlayerController
{
	GENERATED_BODY()

	public:

    UPROPERTY(EditDefaultsOnly, Category = "Variables")
    TMap<TSubclassOf<ASWeaponBase>, FsingleWeaponParams> weaponParameters;

    UPROPERTY(EditDefaultsOnly, Category = "Variables")
    TMap<TEnumAsByte<AmmoType>, int32> ammoMap;
	
};
