// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCharacter.h"
#include "SInteractInterface.h"
#include "SAmmoPickup.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UStaticMesh;


UENUM()
enum class ELocalAmmoType : uint8
{
	Pistol       UMETA(DisplayName = "Pistol Ammo"),
	Rifle        UMETA(DisplayName = "Rifle Ammo"),
	Shotgun      UMETA(DisplayName = "Shotgun Ammo"),
	Special		 UMETA(DisplayName = "Special Ammo"),
};

UENUM()
enum class EAmmoAmount : uint8
{
	Low    		UMETA(DisplayName="Low Ammo"),
	Medium 		UMETA(DisplayName="Medium Ammo"),
	High 		UMETA(DisplayName="High Ammo"),
};

UCLASS()
class ISOLATION_API ASAmmoPickup : public AActor, public ISInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAmmoPickup();

	// Interface functions
	virtual void Interact() override;

	UPROPERTY()
	USceneComponent* ArrowComp;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* PreviewMeshComp;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* LowMeshComp;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* MediumMeshComp;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* HighMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Values")
	int LowAmmoCount;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Values")
	int MediumAmmoCount;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Values")
	int HighAmmoCount;

	UPROPERTY(EditAnywhere, Category = "Ammo Amount")
    EAmmoAmount AmmoAmount;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Type")
	ELocalAmmoType AmmoType;

	int UpdateAmmo;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
