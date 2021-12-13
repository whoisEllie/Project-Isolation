// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractedActor.h"

// Sets default values
ASInteractedActor::ASInteractedActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASInteractedActor::Interacted()
{
	OnInteracted();
}

// Called when the game starts or when spawned
void ASInteractedActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASInteractedActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

