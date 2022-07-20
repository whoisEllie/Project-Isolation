// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "FPSCharacter.h"
#include "FPSCharacterController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASAmmoPickup::ASAmmoPickup()
{
	// Creating our mesh
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshComp"));
	RootComponent = MeshComp;

	// True by default
	bIsEmpty = false;
}

// Called when the game starts or when spawned
void ASAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	// Checking whether the desired mesh exists
	if (AmmoData[AmmoType].FullAmmoBoxes.Find(AmmoAmount) != nullptr)
	{
		// Updating MeshComp with the desired mesh
		MeshComp->SetStaticMesh(AmmoData[AmmoType].FullAmmoBoxes[AmmoAmount]);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh to spawn not found in SAmmoPickup, make sure all meshes are set."));
	}
}

void ASAmmoPickup::Interact()
{
	if (!bIsEmpty)
	{
		// Casting to the player controller (which stores all the ammunition and health variables)
		const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());


		// Printing the ammo before pickup for debug reasons
		if (bDrawDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, FString::FromInt(CharacterController->AmmoMap[AmmoType]));
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, TEXT("Before"));
		}

		// Adding ammo to our character's ammo map
		CharacterController->AmmoMap[AmmoType] += AmmoData[AmmoType].AmmoCounts[AmmoAmount];

		// Printing the ammo after pickup for debug reasons
		if (bDrawDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::FromInt(CharacterController->AmmoMap[AmmoType]));
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, TEXT("After"));
		}

		// Spawning our pickup sound effect
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PickupSFX, GetActorLocation());

		// Switching the mesh to it's empty variant
		if (!bInfinite)
		{
			SetEmptyMesh();
		}
	}
}
	
void ASAmmoPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Checking whether the desired mesh exists in the editor
	if (AmmoData[AmmoType].FullAmmoBoxes.Find(AmmoAmount) != nullptr)
	{
		// Updating MeshComp with the desired mesh in editor
		MeshComp->SetStaticMesh(AmmoData[AmmoType].FullAmmoBoxes[AmmoAmount]);
	}
}

void ASAmmoPickup::SetEmptyMesh()
{
	// Checking whether the desired empty mesh exists
	if (AmmoData[AmmoType].EmptyAmmoBoxes.Find(AmmoAmount) != nullptr)
	{
		// Updating MeshComp with the desired empty mesh
		MeshComp->SetStaticMesh(AmmoData[AmmoType].EmptyAmmoBoxes[AmmoAmount]);
	}

	// Updating bIsEmpty
	bIsEmpty = true;
}

