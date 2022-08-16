// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#include "EnhancedInputComponent.h"
#include "FPSCharacter.h"
#include "FPSCharacterController.h"
#include "WeaponBase.h"
#include "WeaponPickup.h"
#include "Camera/CameraComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
		
}

// Swapping weapons with the scroll wheel
void UInventoryComponent::ScrollWeapon(const FInputActionValue& Value)
{
	int NewID;
    
	if (Value[0] < 0)
	{        
		NewID = FMath::Clamp(CurrentWeaponSlot + 1, 0, NumberOfWeaponSlots - 1);

		if (CurrentWeaponSlot == NumberOfWeaponSlots - 1)
		{
			NewID = 0;
		}
	}
	else
	{        
		NewID = FMath::Clamp(CurrentWeaponSlot - 1, 0, NumberOfWeaponSlots - 1);

		if (CurrentWeaponSlot == 0)
		{
			NewID = NumberOfWeaponSlots - 1;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Wheee"));
	SwapWeapon(NewID);
}

void UInventoryComponent::SwapWeapon(int SlotId)
{
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, FString::FromInt(SlotId));

    if (CurrentWeaponSlot == SlotId) { return; }
    if (!EquippedWeapons.Contains(SlotId)) { return; }
    
    CurrentWeaponSlot = SlotId;
    
    if (CurrentWeapon)
    {
        CurrentWeapon->PrimaryActorTick.bCanEverTick = false;
        CurrentWeapon->SetActorHiddenInGame(true);
        CurrentWeapon->StopFire();
    }
    
    CurrentWeapon = EquippedWeapons[SlotId];
    if (CurrentWeapon)
    {
        CurrentWeapon->PrimaryActorTick.bCanEverTick = true;
        CurrentWeapon->SetActorHiddenInGame(false);
        if (CurrentWeapon->WeaponData.WeaponEquip)
        {
        	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
        	{
        		FPSCharacter->GetHandsMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponEquip, 1.0f);
        	}
        }
    }
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void UInventoryComponent::UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, int InventoryPosition, bool bSpawnPickup,
                      bool bStatic, FTransform PickupTransform, FRuntimeWeaponData DataStruct)
{
    // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (InventoryPosition == CurrentWeaponSlot && EquippedWeapons.Contains(InventoryPosition))
    {
        if (bSpawnPickup)
        {
            // Calculating the location where to spawn the new weapon in front of the player

        	FVector TraceStart = FVector::ZeroVector;
        	FRotator TraceStartRotation = FRotator::ZeroRotator;
        	
        	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
        	{
        		TraceStart = FPSCharacter->GetCameraComponent()->GetComponentLocation();
        		TraceStartRotation = FPSCharacter->GetCameraComponent()->GetComponentRotation();
        	}
            const FVector TraceDirection = TraceStartRotation.Vector();
            const FVector TraceEnd = TraceStart + TraceDirection * WeaponSpawnDistance;

            // Spawning the new pickup
            ASWeaponPickup* NewPickup = GetWorld()->SpawnActor<ASWeaponPickup>(CurrentWeapon->WeaponData.PickupReference, TraceEnd, FRotator::ZeroRotator, SpawnParameters);
            if (bStatic)
            {
                NewPickup->MainMesh->SetSimulatePhysics(false);
                NewPickup->SetActorTransform(PickupTransform);
            }
            // Applying the current weapon data to the pickup
            NewPickup->bStatic = bStatic;
            NewPickup->bRuntimeSpawned = true;
            NewPickup->WeaponReference = EquippedWeapons[InventoryPosition]->GetClass();
            NewPickup->DataStruct = EquippedWeapons[InventoryPosition]->GeneralWeaponData;
            NewPickup->SpawnAttachmentMesh();
            EquippedWeapons[InventoryPosition]->Destroy();
        }
    }
    // Spawns the new weapon and sets the player as it's owner
    ASWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<ASWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (SpawnedWeapon)
    {
        SpawnedWeapon->SetOwner(GetOwner());
    	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
    	{
    		SpawnedWeapon->AttachToComponent(FPSCharacter->GetHandsMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnedWeapon->WeaponData.WeaponAttachmentSocketName);
    	}
        SpawnedWeapon->GeneralWeaponData = DataStruct;
        SpawnedWeapon->SpawnAttachments();
        EquippedWeapons.Add(InventoryPosition, SpawnedWeapon);

        if (CurrentWeapon)
        {
            CurrentWeapon->PrimaryActorTick.bCanEverTick = false;
            CurrentWeapon->SetActorHiddenInGame(true);
        }
    
        CurrentWeapon = EquippedWeapons[InventoryPosition];
        CurrentWeaponSlot = InventoryPosition; 
        
        if (CurrentWeapon)
        {
            CurrentWeapon->PrimaryActorTick.bCanEverTick = true;
            CurrentWeapon->SetActorHiddenInGame(false);
            if (CurrentWeapon->WeaponData.WeaponEquip)
            {
            	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
	            {
		            FPSCharacter->GetHandsMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponEquip, 1.0f);
	            }
            }
        }
    }
}

FText UInventoryComponent::GetCurrentWeaponRemainingAmmo()
{
	{
		if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
		{
			ASCharacterController* CharacterController = Cast<ASCharacterController>(FPSCharacter->GetController());

			if (CharacterController)
			{
				if (CurrentWeapon != nullptr)
				{
					return FText::AsNumber(CharacterController->AmmoMap[CurrentWeapon->GeneralWeaponData.AmmoType]);
				}
				return FText::AsNumber(0);
			}
			return FText::FromString("No Character Controller found");
		}

		return FText::FromString("No Character Controller found");
	}
}

// Passing player inputs to WeaponBase
void UInventoryComponent::StartFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

// Passing player inputs to WeaponBase
void UInventoryComponent::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

// Passing player inputs to WeaponBase
void UInventoryComponent::Reload()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UInventoryComponent::SetupInputComponent(UEnhancedInputComponent* PlayerInputComponent)
{
	if (FiringAction)
	{
		// Firing
		PlayerInputComponent->BindAction(FiringAction, ETriggerEvent::Started, this, &UInventoryComponent::StartFire);
		PlayerInputComponent->BindAction(FiringAction, ETriggerEvent::Completed, this, &UInventoryComponent::StopFire);
	}
        
	if (PrimaryWeaponAction)
	{
		// Switching to the primary weapon
		PlayerInputComponent->BindAction(PrimaryWeaponAction, ETriggerEvent::Started, this, &UInventoryComponent::SwapWeapon<0>);
	}

	if (SecondaryWeaponAction)
	{            
		// Switching to the secondary weapon
		PlayerInputComponent->BindAction(SecondaryWeaponAction, ETriggerEvent::Started, this, &UInventoryComponent::SwapWeapon<1>);
	}

	if (ReloadAction)
	{
		// Reloading
		PlayerInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &UInventoryComponent::Reload);
	}

	if (ScrollAction)
	{
		// Scrolling through weapons
		PlayerInputComponent->BindAction(ScrollAction, ETriggerEvent::Started, this, &UInventoryComponent::ScrollWeapon);
	}
}
