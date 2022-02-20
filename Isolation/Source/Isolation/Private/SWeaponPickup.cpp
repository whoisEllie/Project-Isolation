// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponPickup.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

void ASWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
}

ASWeaponPickup::ASWeaponPickup()
{
	MainMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Main Mesh"));
	MainMesh->SetupAttachment(RootComponent);

	BarrelAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BarrelAttachment"));
	MainMesh->SetupAttachment(RootComponent);

	MagazineAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MagazineAttachment"));
	MainMesh->SetupAttachment(RootComponent);
	
	SightsAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SightsAttachment"));
	MainMesh->SetupAttachment(RootComponent);
	
	StockAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StockAttachment"));
	MainMesh->SetupAttachment(RootComponent);
	
	GripAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GripAttachment"));
	MainMesh->SetupAttachment(RootComponent);

	
}

void ASWeaponPickup::Interact()
{
	Super::Interact();

	DataStruct.WeaponAttachments = AttachmentArray;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Picked up weapon"));

	ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
	if ((PlayerCharacter->PrimaryWeapon == nullptr && PlayerCharacter->SecondaryWeapon == nullptr) || (PlayerCharacter->PrimaryWeapon == nullptr && PlayerCharacter->SecondaryWeapon != nullptr))
	{
		PlayerCharacter->PrimaryWeaponCacheMap = DataStruct;
		PlayerCharacter->bNewPrimarySpawn = true;

		
		PlayerCharacter->UpdateWeapon(WeaponReference);
		if (PlayerCharacter->CurrentWeapon)
		{
			PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->PrimaryWeaponCacheMap.WeaponAttachments);
		}
		PlayerCharacter->PrimaryWeapon = WeaponReference;
		PlayerCharacter->bIsPrimary = true;

		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("New Primary"));
	}
	else if (PlayerCharacter->SecondaryWeapon == nullptr && PlayerCharacter->PrimaryWeapon != nullptr)
	{
		PlayerCharacter->SecondaryWeaponCacheMap = DataStruct;
		PlayerCharacter->bNewSecondarySpawn = true;
		
		PlayerCharacter->UpdateWeapon(WeaponReference);
		if (PlayerCharacter->CurrentWeapon)
		{
			PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->SecondaryWeaponCacheMap.WeaponAttachments);
		}
		PlayerCharacter->SecondaryWeapon = WeaponReference;
		PlayerCharacter->bIsPrimary = false;
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("New Secondary"));
	}
	else
	{
		if (PlayerCharacter->bIsPrimary)
		{
			PlayerCharacter->PrimaryWeaponCacheMap = DataStruct;
			PlayerCharacter->bNewPrimarySpawn = true;
			
			PlayerCharacter->UpdateWeapon(WeaponReference);
			if (PlayerCharacter->CurrentWeapon)
			{
				PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->PrimaryWeaponCacheMap.WeaponAttachments);
			}
			PlayerCharacter->PrimaryWeapon = WeaponReference;
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("New Primary"));
		}
		else
		{
			PlayerCharacter->SecondaryWeaponCacheMap = DataStruct;
			PlayerCharacter->bNewSecondarySpawn = true;
			
			PlayerCharacter->UpdateWeapon(WeaponReference);
			if (PlayerCharacter->CurrentWeapon)
			{
				PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->SecondaryWeaponCacheMap.WeaponAttachments);
			}
			PlayerCharacter->SecondaryWeapon = WeaponReference;
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("New Secondary"));
		}
	}
}
