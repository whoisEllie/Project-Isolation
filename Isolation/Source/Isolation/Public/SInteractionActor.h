// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SInteractInterface.h"
#include "GameFramework/Actor.h"
#include "SInteractionActor.generated.h"

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

	UPROPERTY(EditAnywhere, Category = "Interacted Actor")
	TArray<ASInteractedActor*> InteractedActors;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called for every item interacted with
	UFUNCTION(BlueprintImplementableEvent)
	void OnInteraction(AActor* ImplementedActor);

	// Called when the interaction is completed
	UFUNCTION(BlueprintImplementableEvent)
	void InteractionCompleted();

};
