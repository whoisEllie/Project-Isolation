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

		FRuntimeWeaponData DataStruct;
    	
    	DataStruct.WeaponAttachments = FAttachmentHelpers::ReplaceIncompatibleAttachments(CurrentWeapon->GetStaticWeaponData()->AttachmentsDataTable, FAttachmentHelpers::RandomiseAllAttachments(CurrentWeapon->GetStaticWeaponData()->AttachmentsDataTable));

    	// Applying default values if the weapon doesn't use attachments
		DataStruct.AmmoType = CurrentWeapon->GetStaticWeaponData()->AmmoToUse;
		DataStruct.ClipCapacity = CurrentWeapon->GetStaticWeaponData()->ClipCapacity;
		DataStruct.ClipSize = CurrentWeapon->GetStaticWeaponData()->ClipSize;
		DataStruct.WeaponHealth = FMath::FRandRange(10.0f, 75.0f);
    	
    	// Getting a reference to our Weapon Data table in order to see if we have attachments
        if (const FStaticWeaponData* WeaponData = CurrentWeapon->GetWeaponDataTable()->FindRow<FStaticWeaponData>(FName(CurrentWeapon->GetDataTableNameRef()), FString(CurrentWeapon->GetDataTableNameRef()),true))
        {
	        // Spawning attachments if the weapon has them and the attachments table exists
	        if (WeaponData->bHasAttachments && CurrentWeapon->GetStaticWeaponData()->AttachmentsDataTable)
	        {
		        // Iterating through all the attachments in AttachmentArray
		        for (FName RowName : DataStruct.WeaponAttachments)
		        {
			        // Searching the data table for the attachment
			        const FAttachmentData* AttachmentData = CurrentWeapon->GetStaticWeaponData()->AttachmentsDataTable->FindRow<FAttachmentData>(
				        RowName, RowName.ToString(), true);

			        // Applying the effects of the attachment
			        if (AttachmentData)
			        {
				        if (AttachmentData->AttachmentType == EAttachmentType::Barrel)
				        {
				        }
				        else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
				        {
					        // Pulling default values from the Magazine attachment type
						    DataStruct.AmmoType = AttachmentData->AmmoToUse;
						    DataStruct.ClipCapacity = AttachmentData->ClipCapacity;
						    DataStruct.ClipSize = AttachmentData->ClipSize;
						    DataStruct.WeaponHealth = FMath::FRandRange(10.0f, 75.0f);
				        }
			        }
		        }
	        }
		
        }
    	CurrentWeapon->SetRuntimeWeaponData(DataStruct);
        CurrentWeapon->SpawnAttachments();

    	//TODO: Visibility check for AI, to make sure the player is still visible
    	//TODO: AI Request firing tokens from AI subsystem
    	//TODO: Handle AI dropping weapon pickups when they die
    }
}

void AAICharacter::StartFire()
{
	CurrentWeapon->ScheduleAiFire();
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