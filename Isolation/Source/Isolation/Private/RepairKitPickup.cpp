// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairKitPickup.h"

#include "FPSCharacter.h"
#include "FPSCharacterController.h"
#include "Components/WidgetManagementComponent.h"
#include "Kismet/GameplayStatics.h"


void ARepairKitPickup::Interact()
{
	Super::Interact();

	const AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	AFPSCharacterController* CharacterController = Cast<AFPSCharacterController>(PlayerCharacter->GetController());

	CharacterController->AmmoBoxCount += 1;
	
	// Spawning our pickup sound effect
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PickupSFX, GetActorLocation());

	if (const UWidgetManagementComponent* WidgetManagementComponent = PlayerCharacter->FindComponentByClass<UWidgetManagementComponent>())
	{
		WidgetManagementComponent->GetPlayerHud()->ShowRepairKitCount();
	}
	
	Destroy();
}

void ARepairKitPickup::BeginPlay()
{
	Super::BeginPlay();

	InteractionText = FText::FromString("Weapon Repair Kit");
}
