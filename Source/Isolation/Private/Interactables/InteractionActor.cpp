// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/InteractionActor.h"
#include "InteractedActor.h"

// Implementing the function from our interface
void AInteractionActor::Interact(AActor* InteractionDelegate)
{

	Super::Interact(InteractionDelegate);
	
	// Calling Interacted() on all of the interacted actors
	for (AInteractedActor* InteractedActor : InteractedActors)
	{
		InteractedActor->Interacted();
		OnInteraction(InteractedActor);
	}

	// Calling the local interaction complete function
	InteractionCompleted();
}