// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"
#include "EnhancedInputComponent.h"
#include "FPSCharacter.h"
#include "FPSCharacterController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "WeaponBase.h"
#include "Interactables/WeaponPickup.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMeshActor.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
}

// Swapping weapons with the scroll wheel
void UInventoryComponent::ScrollWeapon(const FInputActionValue& Value)
{
	int NewID;

	// Value[0] determines the axis of input for our scroll wheel
	// a positive value indicates scrolling towards you, while a negative one
	// represents scrolling away from you
	
	if (Value[0] < 0)
	{
		NewID = FMath::Clamp(CurrentWeaponSlot + 1, 0, NumberOfWeaponSlots - 1);

		// If we've reached the end of the weapons array, loop back around to index 0
		if (CurrentWeaponSlot == NumberOfWeaponSlots - 1)
		{
			NewID = 0;
		}
	}
	else
	{        
		NewID = FMath::Clamp(CurrentWeaponSlot - 1, 0, NumberOfWeaponSlots - 1);

		// If we've reached index 0, loop back around to the max index
		if (CurrentWeaponSlot == 0)
		{
			NewID = NumberOfWeaponSlots - 1;
		}
	}
	
	SwapWeapon(NewID);
}

void UInventoryComponent::BeginDestroyCurrentWeapon(UAnimMontage* WeaponDestroyedHandsAnim, UNiagaraSystem* WeaponDestroyedParticleSystem)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();

		if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
		{
			// Playing animation montage
			FPSCharacter->GetHandsMesh()->GetAnimInstance()->Montage_Play(WeaponDestroyedHandsAnim);
			
			// Spawning the particle system
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponDestroyedParticleSystem,
			                                               FPSCharacter->GetHandsMesh()->GetSocketLocation(
				                                               CurrentWeapon->GetStaticWeaponData()->
				                                                              WeaponAttachmentSocketName),
			                                               FRotator::ZeroRotator);

			// Updating the weapon meshes
			const float AnimTime = FPSCharacter->GetHandsMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->GetStaticWeaponData()->WeaponDestroyedHandsAnim, 1.0f); 
        	GetWorld()->GetTimerManager().SetTimer(DestroyWeapon, this, &UInventoryComponent::DestroyCurrentWeapon, AnimTime-0.1f, false, AnimTime-0.1f);
		}
	}
}

void UInventoryComponent::SwapWeapon(const int SlotId)
{
	// Returning if the target weapon is already equipped or it does not exist
    if (CurrentWeaponSlot == SlotId) { return; }
    if (!EquippedWeapons.Contains(SlotId)) { return; }
	
    CurrentWeaponSlot = SlotId;

	// Disabling the currently equipped weapon, if it exists
    if (CurrentWeapon)
    {
        CurrentWeapon->PrimaryActorTick.bCanEverTick = false;
        CurrentWeapon->SetActorHiddenInGame(true);
        CurrentWeapon->StopFire();
    }

	// Swapping to the new weapon, enabling it and playing it's equip animation
    CurrentWeapon = EquippedWeapons[SlotId];
    if (CurrentWeapon)
    {
        CurrentWeapon->PrimaryActorTick.bCanEverTick = true;
        CurrentWeapon->SetActorHiddenInGame(false);
        if (CurrentWeapon->GetStaticWeaponData()->WeaponEquip)
        {
        	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
        	{
	            FPSCharacter->GetHandsMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->GetStaticWeaponData()->WeaponEquip, 1.0f);
        	}
        }
    }
}

void UInventoryComponent::DestroyCurrentWeapon()
{
	const int* CurrentWeaponId = EquippedWeapons.FindKey(CurrentWeapon);
	EquippedWeapons.Remove(*CurrentWeaponId);
	CurrentWeapon->Destroy();
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void UInventoryComponent::UpdateWeapon(const TSubclassOf<AWeaponBase> NewWeapon, const int InventoryPosition, const bool bSpawnPickup,
                                       const bool bStatic, const FTransform PickupTransform, const FRuntimeWeaponData DataStruct)
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
            AWeaponPickup* NewPickup = GetWorld()->SpawnActor<AWeaponPickup>(CurrentWeapon->GetStaticWeaponData()->PickupReference, TraceEnd, FRotator::ZeroRotator, SpawnParameters);
            if (bStatic)
            {
                NewPickup->GetMainMesh()->SetSimulatePhysics(false);
                NewPickup->SetActorTransform(PickupTransform);
            }
            // Applying the current weapon data to the pickup
            NewPickup->SetStatic(bStatic);
            NewPickup->SetRuntimeSpawned(true);
            NewPickup->SetWeaponReference(EquippedWeapons[InventoryPosition]->GetClass());
            NewPickup->SetCacheDataStruct(EquippedWeapons[InventoryPosition]->GetRuntimeWeaponData());
            NewPickup->SpawnAttachmentMesh();
            EquippedWeapons[InventoryPosition]->Destroy();
        }
    }

    // Spawns the new weapon and sets the player as it's owner
    AWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (SpawnedWeapon)
    {
    	// Placing the new weapon at the correct location and finishing up it's initialisation
        SpawnedWeapon->SetOwner(GetOwner());
    	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
    	{
    		SpawnedWeapon->AttachToComponent(FPSCharacter->GetHandsMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnedWeapon->GetStaticWeaponData()->WeaponAttachmentSocketName);
    	}
        SpawnedWeapon->SetRuntimeWeaponData(DataStruct);
        SpawnedWeapon->SpawnAttachments();
        EquippedWeapons.Add(InventoryPosition, SpawnedWeapon);

		// Disabling the currently equipped weapon, if it exists
        if (CurrentWeapon)
        {
            CurrentWeapon->PrimaryActorTick.bCanEverTick = false;
            CurrentWeapon->SetActorHiddenInGame(true);
        	CurrentWeapon->StopFire();
        }
    	
    	// Swapping to the new weapon, enabling it and playing it's equip animation
        CurrentWeapon = EquippedWeapons[InventoryPosition];
        CurrentWeaponSlot = InventoryPosition; 
        
        if (CurrentWeapon)
        {
            CurrentWeapon->PrimaryActorTick.bCanEverTick = true;
            CurrentWeapon->SetActorHiddenInGame(false);
            if (CurrentWeapon->GetStaticWeaponData()->WeaponEquip)
            {
            	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
	            {
		            FPSCharacter->GetHandsMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->GetStaticWeaponData()->WeaponEquip, 1.0f);
	            }
            }
        }
    }
}

FText UInventoryComponent::GetCurrentWeaponRemainingAmmo() const
{
	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
	{
		AFPSCharacterController* CharacterController = Cast<AFPSCharacterController>(FPSCharacter->GetController());

		if (CharacterController)	
		{
			if (CurrentWeapon != nullptr)
			{
				return FText::AsNumber(CharacterController->AmmoMap[CurrentWeapon->GetRuntimeWeaponData()->AmmoType]);
			}
			UE_LOG(LogProfilingDebugging, Log, TEXT("Cannot find Current Weapon"));
			return FText::AsNumber(0);
		}
		UE_LOG(LogProfilingDebugging, Error, TEXT("No character controller found in UInventoryComponent"))
		return FText::FromString("Err");
	}
	UE_LOG(LogProfilingDebugging, Error, TEXT("No player character found in UInventoryComponent"))
	return FText::FromString("Err");
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
