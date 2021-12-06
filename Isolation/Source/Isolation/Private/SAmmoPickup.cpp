// Fill out your copyright notice in the Description page of Project Settings.


#include "SAmmoPickup.h"
#include "SCharacter.h"
#include "SCharacterController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASAmmoPickup::ASAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ArrowComp = CreateDefaultSubobject<USceneComponent>(TEXT("ArrowComp"));
	RootComponent = ArrowComp;

	PreviewMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshComp"));
	PreviewMeshComp->SetupAttachment(ArrowComp);

	LowMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LowMeshComp"));
	LowMeshComp->SetupAttachment(ArrowComp);
	LowMeshComp->ToggleVisibility(false);

	MediumMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MediumMeshComp"));
	MediumMeshComp->SetupAttachment(ArrowComp);
	MediumMeshComp->ToggleVisibility(false);

	HighMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighMeshComp"));
	HighMeshComp->SetupAttachment(ArrowComp);
	HighMeshComp->ToggleVisibility(false);

	EmptyLowMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmptyLowMeshComp"));
	EmptyLowMeshComp->SetupAttachment(ArrowComp);
	EmptyLowMeshComp->ToggleVisibility(false);

	EmptyMediumMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmptyMediumMeshComp"));
	EmptyMediumMeshComp->SetupAttachment(ArrowComp);
	EmptyMediumMeshComp->ToggleVisibility(false);

	EmptyHighMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmptyHighMeshComp"));
	EmptyHighMeshComp->SetupAttachment(ArrowComp);
	EmptyHighMeshComp->ToggleVisibility(false);
}

// Called when the game starts or when spawned
void ASAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	PreviewMeshComp->ToggleVisibility(false);

	switch(AmmoAmount)
	{
		case EAmmoAmount::Low:
			LowMeshComp->ToggleVisibility(false);
			UpdateAmmo = LowAmmoCount;
			break;

		case EAmmoAmount::Medium:
			MediumMeshComp->ToggleVisibility(false);
			UpdateAmmo = MediumAmmoCount;
			break;

		case EAmmoAmount::High:
			HighMeshComp->ToggleVisibility(true);
			UpdateAmmo = HighAmmoCount;
			break;
	}

	bCanInteract = true;
}

void ASAmmoPickup::Interact()
{
	if (bCanInteract)
	{
		// Casting to the playercontroller (which stores all the ammunition and health variables)
		ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

		switch (AmmoType)
		{
		case ELocalAmmoType::Pistol:
			CharacterController->ammoMap[EAmmoType::Pistol] += UpdateAmmo;
			if (bDrawDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::FromInt(CharacterController->ammoMap[EAmmoType::Pistol]));
			}
			break;

		case ELocalAmmoType::Rifle:
			CharacterController->ammoMap[EAmmoType::Rifle] += UpdateAmmo;
			if (bDrawDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::FromInt(CharacterController->ammoMap[EAmmoType::Rifle]));
			}
			break;

		case ELocalAmmoType::Shotgun:
			CharacterController->ammoMap[EAmmoType::Shotgun] += UpdateAmmo;
			if (bDrawDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::FromInt(CharacterController->ammoMap[EAmmoType::Shotgun]));
			}
			break;

		case ELocalAmmoType::Special:
			CharacterController->ammoMap[EAmmoType::Special] += UpdateAmmo;
			if (bDrawDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::FromInt(CharacterController->ammoMap[EAmmoType::Special]));
			}
			break;
		}

		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PickupSFX, GetActorLocation());
		UpdateEmptyVisibility();
	}
}

// Called every frame
void ASAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASAmmoPickup::UpdateEmptyVisibility()
{
	switch(AmmoAmount)
	{
	case EAmmoAmount::Low:
		EmptyLowMeshComp->ToggleVisibility(true);
		LowMeshComp->ToggleVisibility(false);
		break;

	case EAmmoAmount::Medium:
		EmptyMediumMeshComp->ToggleVisibility(true);
		MediumMeshComp->ToggleVisibility(false);
		break;

	case EAmmoAmount::High:
		EmptyHighMeshComp->ToggleVisibility(true);
		HighMeshComp->ToggleVisibility(false);
		break;
	}

	bCanInteract = false;
}

