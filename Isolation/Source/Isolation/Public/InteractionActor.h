// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "GameFramework/Actor.h"
#include "InteractionActor.generated.h"

class ASInteractedActor;
class UStaticMeshComponent;

UCLASS()
class ISOLATION_API ASInteractionActor : public AActor, public ISInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASInteractionActor();

	virtual void Interact() override;

	// The actors to which we cast upon receiving an interaction
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interacted Actor")
	TArray<ASInteractedActor*> InteractedActors;

	// The description to be passed to the player for this item
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Interacted Actor")
	FText PopupDescription;

	// The mesh as which to render
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MeshComp;
	
	// Called for every item interacted with
	UFUNCTION(BlueprintImplementableEvent)
	void OnInteraction(AActor* ImplementedActor);

	// Called when the interaction is completed
	UFUNCTION(BlueprintImplementableEvent)
	void InteractionCompleted();

};
