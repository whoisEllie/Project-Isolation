// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairKitPickup.h"

#include "FPSCharacter.h"
#include "FPSCharacterController.h"
#include "Kismet/GameplayStatics.h"


void ASRepairKitPickup::Interact()
{
	Super::Interact();

	const AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

	CharacterController->AmmoBoxCount += 1;
	
	// Spawning our pickup sound effect
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PickupSFX, GetActorLocation());

	PlayerCharacter->GetPlayerHud()->ShowRepairKitCount();
	
	Destroy();
}

void ASRepairKitPickup::BeginPlay()
{
	Super::BeginPlay();

	PopupDescription = FText::FromString("Weapon Repair Kit");
}
