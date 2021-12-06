// Fill out your copyright notice in the Description page of Project Settings.


#include "SAmmoPickup.h"
#include "SCharacter.h"
#include "SCharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ASAmmoPickup::ASAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ArrowComp = CreateDefaultSubobject<USceneComponent>(TEXT("arrowComp"));
	RootComponent = ArrowComp;

	PreviewMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("previewMeshComp"));
	PreviewMeshComp->SetupAttachment(ArrowComp);

	LowMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("lowMeshComp"));
	LowMeshComp->SetupAttachment(ArrowComp);
	LowMeshComp->ToggleVisibility(false);

	MediumMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mediumMeshComp"));
	MediumMeshComp->SetupAttachment(ArrowComp);
	MediumMeshComp->ToggleVisibility(false);

	HighMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("highMeshComp"));
	HighMeshComp->SetupAttachment(ArrowComp);
	HighMeshComp->ToggleVisibility(false);
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
	
}

void ASAmmoPickup::Interact()
{
    // Casting to the playercontroller (which stores all the ammunition and health variables)
    ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

    switch (AmmoType)
    {
    case ELocalAmmoType::Pistol:
    	CharacterController->ammoMap[EAmmoType::Pistol] += UpdateAmmo;
    	break;

    case ELocalAmmoType::Rifle:
    	CharacterController->ammoMap[EAmmoType::Rifle] += UpdateAmmo;
    	break;

    case ELocalAmmoType::Shotgun:
    	CharacterController->ammoMap[EAmmoType::Shotgun] += UpdateAmmo;
    	break;

    case ELocalAmmoType::Special:
    	CharacterController->ammoMap[EAmmoType::Special] += UpdateAmmo;
    	break;
    }

	Destroy();
}

// Called every frame
void ASAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

