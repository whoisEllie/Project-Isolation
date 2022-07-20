// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSCharacter.h"
#include "InteractionActor.h"
#include "WeaponPickup.generated.h"

class UStaticMeshComponent;
class UDataTable;
class ASWeaponBase;

UCLASS()
class ISOLATION_API ASWeaponPickup : public AActor, public ISInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeaponPickup();

	// Interface function
	virtual void Interact() override;

	// Spawns attachment meshes from data table
	UFUNCTION(BlueprintCallable)
	void SpawnAttachmentMesh();

	// Weapon to spawn when picked up
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASWeaponBase> WeaponReference;

	// Local weapon data struct to keep track of ammo amounts and weapon health
	UPROPERTY()
	FWeaponDataStruct DataStruct;

	// Data table reference for weapon (used to see if the weapon has attachments)
	UPROPERTY(EditDefaultsOnly, Category = "Data Table")
	UDataTable* WeaponDataTable;

	// Data table reference for attachments
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data Table")
	UDataTable* AttachmentsDataTable;

	// Whether this pickup has been spawned at runtime or not  (determines whether we inherit DataStruct values or
	// reset to default)
	bool bRuntimeSpawned;

	// Whether to run physics simulations or not
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	bool bStatic;

	// The array of attachments to spawn (usually inherited, can be set by instance)
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Data")
	TArray<FName> AttachmentArray;

	// Whether this weapon will become the primary or secondary weapon
	bool bIsNewPrimary;
	
	// Visualisation name
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	FText WeaponName;

	// Main weapon mesh
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MainMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Meshes for Attachments

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* BarrelAttachment;

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* MagazineAttachment;

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* SightsAttachment;
	
	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* StockAttachment;

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* GripAttachment;
};
