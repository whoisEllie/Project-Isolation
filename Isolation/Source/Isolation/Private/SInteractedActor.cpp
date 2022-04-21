// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractedActor.h"

// Sets default values
ASInteractedActor::ASInteractedActor()
{
}

void ASInteractedActor::Interacted()
{
	// Calls function to be implemented in blueprints
	OnInteracted();
}

// Called when the game starts or when spawned
void ASInteractedActor::BeginPlay()
{
	Super::BeginPlay();
	
}