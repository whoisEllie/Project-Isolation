// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponPickup.h"
#include "SWeaponBase.h"
#include "func_lib/SAttachmentRandomiser.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASWeaponPickup::ASWeaponPickup()
{
	// Creating all of our 
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	MainMesh->SetRenderCustomDepth(true);
	MainMesh->SetCustomDepthStencilValue(2);
	MainMesh->SetupAttachment(RootComponent);

	BarrelAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelAttachment"));
	BarrelAttachment->SetRenderCustomDepth(true);
	BarrelAttachment->SetCustomDepthStencilValue(2);
	BarrelAttachment->SetupAttachment(MainMesh);

	MagazineAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineAttachment"));
	MagazineAttachment->SetRenderCustomDepth(true);
	MagazineAttachment->SetCustomDepthStencilValue(2);
	MagazineAttachment->SetupAttachment(MainMesh);
	
	SightsAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SightsAttachment"));
	SightsAttachment->SetRenderCustomDepth(true);
	SightsAttachment->SetCustomDepthStencilValue(2);
	SightsAttachment->SetupAttachment(MainMesh);

	StockAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StockAttachment"));
	StockAttachment->SetRenderCustomDepth(true);
	StockAttachment->SetCustomDepthStencilValue(2);
	StockAttachment->SetupAttachment(MainMesh);
	
	GripAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GripAttachment"));
	GripAttachment->SetRenderCustomDepth(true);
	GripAttachment->SetCustomDepthStencilValue(2);
	GripAttachment->SetupAttachment(MainMesh);
}

// Called when the game starts or when spawned
void ASWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	// Spawning attachments on begin play
	SpawnAttachmentMesh();

	// Simulating physics if not bStatic
	if (!bStatic)
	{
		MainMesh->SetSimulatePhysics(true);
	}
}

void ASWeaponPickup::SpawnAttachmentMesh()
{
	// Getting a reference to our Weapon Data table in order to see if we have attachments
	ASWeaponBase* WeaponVariables =  WeaponReference.GetDefaultObject();
	const FWeaponData* WeaponData = WeaponDataTable->FindRow<FWeaponData>(FName(WeaponVariables->DataTableNameRef), FString(WeaponVariables->DataTableNameRef), true);

	// Spawning attachments if the weapon has them and the attachments table exists
	if (WeaponData->bHasAttachments && AttachmentsDataTable)
	{
		// Iterating through all the attachments in AttachmentArray
		for (FName RowName : AttachmentArray)
		{
			// Searching the data table for the attachment
			const FAttachmentData* AttachmentData = AttachmentsDataTable->FindRow<FAttachmentData>(RowName, RowName.ToString(), true);

			// Applying the effects of the attachment
			if (AttachmentData)
			{
				if (AttachmentData->AttachmentType == EAttachmentType::Barrel)
				{
					BarrelAttachment->SetStaticMesh(AttachmentData->PickupMesh);
				}
				else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
				{
					MagazineAttachment->SetStaticMesh(AttachmentData->PickupMesh);
					// Pulling default values from the Magazine attachment type
					if (!bRuntimeSpawned)
					{
						DataStruct.AmmoType = AttachmentData->AmmoToUse;
						DataStruct.ClipCapacity = AttachmentData->ClipCapacity;
						DataStruct.ClipSize = AttachmentData->ClipSize;
						DataStruct.WeaponHealth = 100.0f;
					}
				}
				else if (AttachmentData->AttachmentType == EAttachmentType::Sights)
				{
					SightsAttachment->SetStaticMesh(AttachmentData->PickupMesh);
				}
				else if (AttachmentData->AttachmentType == EAttachmentType::Stock)
				{
					StockAttachment->SetStaticMesh(AttachmentData->PickupMesh);
				}
				else if (AttachmentData->AttachmentType == EAttachmentType::Grip)
				{
					GripAttachment->SetStaticMesh(AttachmentData->PickupMesh);
				}
			}
		}
	}
	else
	{
		// Applying default values if the weapon doesn't use attachments
		if (!bRuntimeSpawned)
		{
			DataStruct.AmmoType = WeaponData->AmmoToUse;
			DataStruct.ClipCapacity = WeaponData->ClipCapacity;
			DataStruct.ClipSize = WeaponData->ClipSize;
			DataStruct.WeaponHealth = 100.0f;
		}
	}
}

void ASWeaponPickup::Interact()
{
	// Casting to the player character 
	DataStruct.WeaponAttachments = AttachmentArray;
	ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Swapping weapons based on different situations
	
	// If the player has no weapons or only a primary weapon
	if ((PlayerCharacter->PrimaryWeapon == nullptr && PlayerCharacter->SecondaryWeapon == nullptr) || (PlayerCharacter->PrimaryWeapon == nullptr && PlayerCharacter->SecondaryWeapon != nullptr))
	{
		PlayerCharacter->PrimaryWeaponCacheMap = DataStruct;
		
		PlayerCharacter->UpdateWeapon(WeaponReference, false, DataStruct, bStatic, FTransform::Identity);
		if (PlayerCharacter->CurrentWeapon)
		{
			PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->PrimaryWeaponCacheMap.WeaponAttachments);
		}
		PlayerCharacter->PrimaryWeapon = WeaponReference;
		PlayerCharacter->bIsPrimary = true;
	}
	// If the player has a primary weapon but no secondary weapon
	else if (PlayerCharacter->SecondaryWeapon == nullptr && PlayerCharacter->PrimaryWeapon != nullptr)
	{
		PlayerCharacter->SecondaryWeaponCacheMap = DataStruct;
		
		PlayerCharacter->UpdateWeapon(WeaponReference, false, DataStruct, bStatic, FTransform::Identity);
		if (PlayerCharacter->CurrentWeapon)
		{
			PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->SecondaryWeaponCacheMap.WeaponAttachments);
		}
		PlayerCharacter->SecondaryWeapon = WeaponReference;
		PlayerCharacter->bIsPrimary = false;
	}
	// If both weapon slots are occupied
	else
	{
		// Swapping the primary weapon
		if (PlayerCharacter->bIsPrimary)
		{
			PlayerCharacter->UpdateWeapon(WeaponReference, true, PlayerCharacter->PrimaryWeaponCacheMap, bStatic, GetTransform());
			PlayerCharacter->PrimaryWeaponCacheMap = DataStruct;
			
			if (PlayerCharacter->CurrentWeapon)
			{
				PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->PrimaryWeaponCacheMap.WeaponAttachments);
			}
			PlayerCharacter->PrimaryWeapon = WeaponReference;
		}
		// Swapping the secondary weapon
		else
		{
			PlayerCharacter->UpdateWeapon(WeaponReference, true, PlayerCharacter->SecondaryWeaponCacheMap, bStatic, GetTransform());
			PlayerCharacter->SecondaryWeaponCacheMap = DataStruct;
			
			if (PlayerCharacter->CurrentWeapon)
			{
				PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->SecondaryWeaponCacheMap.WeaponAttachments);
			}
			PlayerCharacter->SecondaryWeapon = WeaponReference;
		}
	}

	if (PlayerCharacter->CurrentWeapon)
	{
		if (PlayerCharacter->CurrentWeapon->WeaponData->WeaponEquip)
		{
			PlayerCharacter->HandsMeshComp->GetAnimInstance()->Montage_Play(PlayerCharacter->CurrentWeapon->WeaponEquip, 1.0f);
		}
	}

	// Destroying the pickup
	Destroy();
}