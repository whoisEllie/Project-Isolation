// Fill out your copyright notice in the Description page of Project Settings.


#include "SAmmoPickup.h"

// Sets default values
ASAmmoPickup::ASAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASAmmoPickup::Interact()
{
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "Pure", true);
}

// Called every frame
void ASAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

