// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCharacter.h"
#include "SInteractInterface.h"
#include "SAmmoPickup.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USoundBase;


// Keeping track of which asset to spawn
UENUM()
enum class EAmmoAmount : uint8
{
	Low    		UMETA(DisplayName="Low Ammo"),
	Medium 		UMETA(DisplayName="Medium Ammo"),
	High 		UMETA(DisplayName="High Ammo"),
};

USTRUCT()
struct FAmmoTypeData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TMap<EAmmoAmount, UStaticMesh*> FullAmmoBoxes;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<EAmmoAmount, UStaticMesh*> EmptyAmmoBoxes;

	UPROPERTY(EditDefaultsOnly)
	TMap<EAmmoAmount, int> AmmoCounts;
	
};

UCLASS()
class ISOLATION_API ASAmmoPickup : public AActor, public ISInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAmmoPickup();

	// Interface function
	virtual void Interact() override;

	// Override construction script
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// The main mesh
	UPROPERTY()
	UStaticMeshComponent* MeshComp;

	// The Map keeping track of all values for the meshes and ammo counts
	UPROPERTY(EditDefaultsOnly, Category = "Meshes")
	TMap<EAmmoType, FAmmoTypeData> AmmoData;
	
	// The enum implementation for what amount of ammunition to spawn 
	UPROPERTY(EditInstanceOnly, Category = "Properties")
    EAmmoAmount AmmoAmount;

	// The enum implementation for what type of ammo this pickup should spawn
	UPROPERTY(EditInstanceOnly, Category = "Properties")
	EAmmoType AmmoType;
	
	// Whether the player can interact with this ammo pickup (whether it is full or empty, basically)
	bool bIsEmpty;

	// Whether debug print statements should be shown
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug;

	// The sound effect to play when ammunition is collected
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases	")
	USoundBase* PickupSFX;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Updates the mesh from full to empty
	void SetEmptyMesh();
};
