// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCharacter.h"
#include "SInteractInterface.h"
#include "SAmmoPickup.generated.h"

class UStaticMeshComponent;
class USceneComponent;


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
	USceneComponent* arrowComp;

	UPROPERTY(EditDefaultsOnly, Category = "Static Meshes")
	UStaticMeshComponent* previewMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Static Meshes")
	UStaticMeshComponent* lowMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Static Meshes")
	UStaticMeshComponent* mediumMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Static Meshes")
	UStaticMeshComponent* highMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Values")
	int lowAmmoCount;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Values")
	int mediumAmmoCount;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo Values")
	int highAmmoCount;

	UPROPERTY(EditAnywhere, Category = "Ammo Amount")
    EAmmoAmount ammoAmount;

	UPROPERTY(EditAnywhere, Category = "Ammo Type")
	ELocalAmmoType ammoType;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
