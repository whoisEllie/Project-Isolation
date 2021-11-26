// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SCharacter.h"
#include "SInteractInterface.h"
#include "SAmmoPickup.generated.h"


UCLASS()
class ISOLATION_API ASAmmoPickup : public AActor, public ISInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASAmmoPickup();

	// Interface functions
	virtual void Interact() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
