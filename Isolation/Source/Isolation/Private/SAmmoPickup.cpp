// Fill out your copyright notice in the Description page of Project Settings.


#include "SAmmoPickup.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ASAmmoPickup::ASAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	previewMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("lowMeshComp"));
	RootComponent = previewMeshComp;

	lowMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("lowMeshComp"));
	lowMeshComp->ToggleVisibility(false);
	lowMeshComp->AttachToComponent(previewMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale);

	mediumMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mediumMeshComp"));
	mediumMeshComp->ToggleVisibility(false);
	mediumMeshComp->AttachToComponent(previewMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale);

	highMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("highMeshComp"));
	highMeshComp->ToggleVisibility(false);
	highMeshComp->AttachToComponent(previewMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Called when the game starts or when spawned
void ASAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	previewMeshComp->ToggleVisibility(false);

	switch(ammoAmount)
	{
		case EAmmoAmount::Low:
			lowMeshComp->ToggleVisibility(false);
			break;

		case EAmmoAmount::Medium:
			mediumMeshComp->ToggleVisibility(false);
			break;

		case EAmmoAmount::High:
			highMeshComp->ToggleVisibility(true);
			break;
	}
	
}

void ASAmmoPickup::Interact()
{
    
}

// Called every frame
void ASAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

