// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "SIsolationGameInstance.h"

// Sets default values
ASWeaponBase::ASWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeaponBase::StartFire()
{

}

void ASWeaponBase::StopFire()
{

}

void ASWeaponBase::Fire()
{
    USIsolationGameInstance* GameInstanceRef = Cast<USIsolationGameInstance>(GEngine->GetWorld()->GetGameInstance());
    
    if (GameInstanceRef->weaponParameters[0].clipSize)
    {
    
    }
}

void ASWeaponBase::Reload()
{

}

// Called every frame
void ASWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

