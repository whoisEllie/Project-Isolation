// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SInteractionActor.h"
#include "SWeaponBase.h"
#include "SWeaponPickup.generated.h"

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

	virtual void Interact() override;

	void SpawnAttachmentMesh();

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MainMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* BarrelAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MagazineAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* SightsAttachment;
	
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* StockAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* GripAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASWeaponBase> WeaponReference;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FWeaponDataStruct DataStruct;

	UPROPERTY(EditDefaultsOnly, Category = "Data Table")
	UDataTable* AttachmentsDataTable;

	FAttachmentData* AttachmentData;
	
	bool bRuntimeSpawned;

	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	bool bStatic;
	
	UPROPERTY(EditInstanceOnly, Category = "Data")
	TArray<FName> AttachmentArray;

	UPROPERTY()
	bool bIsNewPrimary;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called for every item interacted with
	UFUNCTION(BlueprintImplementableEvent)
	void OnInteraction(AActor* ImplementedActor);

	// Called when the interaction is completed
	UFUNCTION()
	void InteractionCompleted();

};
