// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/DocumentPickup.h"

#include "FPSCharacter.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ADocumentPickup::ADocumentPickup()
{
	InteractionText = FText::FromString("Document");
}

void ADocumentPickup::Interact(AActor* InteractionDelegate)
{
	Super::Interact(InteractionDelegate);

	const AFPSCharacter* CharacterRef = Cast<AFPSCharacter>(InteractionDelegate);
	
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;

	if (bReturnToOrigin)
	{
		UKismetSystemLibrary::MoveComponentTo(MeshComp, BaseLocation, BaseRotation, false, false, InterpTime, false, EMoveComponentAction::Type::Move, LatentInfo);
		bReturnToOrigin = false;
	}
	else if (CharacterRef)
	{
		MeshComp->AttachToComponent(CharacterRef->GetDocumentLocationArrow(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		// What do I do here? Design question - lock the player in place or let them move around with a document 🤔
		UKismetSystemLibrary::MoveComponentTo(MeshComp, CharacterRef->GetDocumentLocationArrow()->GetComponentLocation(), CharacterRef->GetDocumentLocationArrow()->GetComponentRotation() , false, false, InterpTime, false, EMoveComponentAction::Type::Move, LatentInfo);
		bReturnToOrigin = true;
	}
}	

// Called when the game starts or when spawned
void ADocumentPickup::BeginPlay()
{
	Super::BeginPlay();
	
	BaseRotation = MeshComp->GetComponentRotation();
	BaseLocation = MeshComp->GetComponentLocation();
}
