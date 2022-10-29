// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrystalElement.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ISOLATION_API ACrystalElement : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACrystalElement();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* CrystalMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USphereComponent* InfluenceRadiusSphere;

private:

	UPROPERTY(EditInstanceOnly, Category = "Crystal Element")
	float InfluenceSphereRadius = 150.0f;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
