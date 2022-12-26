// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacter.h"
#include "func_lib/AttachmentHelpers.h"
#include "AI/AICharacterController.h"

AAICharacter::AAICharacter()
{
	
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (StarterWeapon)
	{
		UpdateWeapon(StarterWeapon);
	}
}

void AAICharacter::UpdateWeapon(const TSubclassOf<AWeaponBase> NewWeapon)
{
	 // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawns the new weapon and sets the enemy AI as it's owner
    CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (CurrentWeapon)
    {
    	// Placing the new weapon at the correct location and finishing up it's initialisation
        CurrentWeapon->SetOwner(this);
    	CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeapon->GetStaticWeaponData()->AiAttachmentSocketName);
    	CurrentWeapon->GetRuntimeWeaponData()->WeaponAttachments = FAttachmentHelpers::ReplaceIncompatibleAttachments(CurrentWeapon->GetStaticWeaponData()->AttachmentsDataTable, FAttachmentHelpers::RandomiseAllAttachments(CurrentWeapon->GetStaticWeaponData()->AttachmentsDataTable));
        CurrentWeapon->SpawnAttachments();
    }
}

void AAICharacter::StartFire(int ShotsToTake)
{
	CurrentWeapon->AiFire(ShotsToTake);
}

void AAICharacter::StopFire()
{
}

void AAICharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	OutLocation = GetMesh()->GetSocketLocation("HeadSocket");
	OutRotation = GetMesh()->GetSocketRotation("HeadSocket");
}