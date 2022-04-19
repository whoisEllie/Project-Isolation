// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionActor.h"

#include "SCharacter.h"
#include "SInteractedActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASInteractionActor::ASInteractionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

}

void ASInteractionActor::Interact()
{
	for (ASInteractedActor* InteractedActor : InteractedActors)
	{
		InteractedActor->Interacted();
		OnInteraction(InteractedActor);
		
	}
	InteractionCompleted();
}

// Called when the game starts or when spawned
void ASInteractionActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASInteractionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

