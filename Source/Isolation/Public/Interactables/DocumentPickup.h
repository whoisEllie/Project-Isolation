// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionBase.h"
#include "DocumentPickup.generated.h"

UCLASS()
class ISOLATION_API ADocumentPickup : public AInteractionBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADocumentPickup();

	virtual void Interact(AActor* InteractionDelegate) override;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	FRotator BaseRotation;

	UPROPERTY()
	FVector BaseLocation;
	
	bool bReturnToOrigin = false;

	UPROPERTY(EditDefaultsOnly, Category = "Interpolation")
	float InterpTime = 1.0f;
};
