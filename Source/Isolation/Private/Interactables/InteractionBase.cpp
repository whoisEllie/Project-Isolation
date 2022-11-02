// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/InteractionBase.h"


// Sets default values
AInteractionBase::AInteractionBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
}