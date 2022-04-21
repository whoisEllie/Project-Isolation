// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionActor.h"

#include "SCharacter.h"
#include "SInteractedActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASInteractionActor::ASInteractionActor()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

// Implementing the function from our interface
void ASInteractionActor::Interact()
{
	// Calling Interacted() on all of the interacted actors
	for (ASInteractedActor* InteractedActor : InteractedActors)
	{
		InteractedActor->Interacted();
		OnInteraction(InteractedActor);
	}

	// Calling the local interaction complete function
	InteractionCompleted();
}