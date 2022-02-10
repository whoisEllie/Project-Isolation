// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponPickup.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

void ASWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
}

ASWeaponPickup::ASWeaponPickup()
{
	MainMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Main Mesh"));
	MainMesh->SetupAttachment(RootComponent);

	BarrelAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BarrelAttachment"));
	BarrelAttachment->AttachToComponent(MainMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);

	MagazineAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MagazineAttachment"));
	MagazineAttachment->AttachToComponent(MainMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
	SightsAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SightsAttachment"));
	SightsAttachment->AttachToComponent(MainMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
	StockAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StockAttachment"));
	StockAttachment->AttachToComponent(MainMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
	
	GripAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GripAttachment"));
	GripAttachment->AttachToComponent(MainMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

void ASWeaponPickup::Interact()
{
	Super::Interact();

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Picked up weapon"));

	ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	PlayerCharacter->UpdateWeapon(WeaponReference);
}
