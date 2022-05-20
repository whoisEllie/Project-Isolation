// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "STooltipPopup.generated.h"

UCLASS()
class ISOLATION_API ASTooltipPopup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASTooltipPopup();

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	UBoxComponent* BoxComponent;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowDebug = false;

	UPROPERTY(EditInstanceOnly, Category = "Variables")
	FText TooltipTitle;

	UPROPERTY(EditInstanceOnly, Category = "Variables")
	FText TooltipDescription;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ResetDisplay();

	FTimerHandle DisplayDelayHandle;

	bool ShouldDisplay = true;

};
