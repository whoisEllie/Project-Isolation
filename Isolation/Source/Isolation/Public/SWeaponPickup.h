// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SInteractionActor.h"
#include "SWeaponBase.h"
#include "SWeaponPickup.generated.h"

class USkeletalMeshComponent;

/**
 * 
 */
UCLASS()
class ISOLATION_API ASWeaponPickup : public ASInteractionActor
{
	GENERATED_BODY()

	ASWeaponPickup();
	
	virtual void Interact() override;

	virtual void BeginPlay() override;

public:

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	USkeletalMeshComponent* MainMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	USkeletalMeshComponent* BarrelAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	USkeletalMeshComponent* MagazineAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	USkeletalMeshComponent* SightsAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	USkeletalMeshComponent* StockAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	USkeletalMeshComponent* GripAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASWeaponBase> WeaponReference;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FWeaponDataStruct DataStruct;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TArray<FName> AttachmentArray;

	UPROPERTY()
	bool bIsNewPrimary;
};
