// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacter.h"

AAICharacter::AAICharacter()
{
	
}

void AAICharacter::BeginPlay()
{
	if (StarterWeapon)
	{
		UpdateWeapon(StarterWeapon);
	}
}

void AAICharacter::UpdateWeapon(const TSubclassOf<ABaseAIWeapon> NewWeapon) const
{
	 // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawns the new weapon and sets the enemy AI as it's owner
    ABaseAIWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ABaseAIWeapon>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (SpawnedWeapon)
    {
    	// Placing the new weapon at the correct location and finishing up it's initialisation
        SpawnedWeapon->SetOwner(GetOwner());
    	SpawnedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnedWeapon->GetAIWeaponData()->WeaponAttachmentSocketName);
        SpawnedWeapon->SpawnAttachments();
    }
}

void AAICharacter::StartFire()
{
	CurrentWeapon->Fire();
}

void AAICharacter::StopFire()
{
}
