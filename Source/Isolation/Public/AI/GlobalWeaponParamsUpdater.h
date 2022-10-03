// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIManager.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GlobalWeaponParamsUpdater.generated.h"

UCLASS()
class ISOLATION_API AGlobalWeaponParamsUpdater : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGlobalWeaponParamsUpdater();

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	UBoxComponent* CollisionBox;

private:

	UPROPERTY(EditInstanceOnly, Category = "Combat Parameters")
	FGlobalCombatParameters GlobalCombatParametersOverride;

	/** Updates the master global combat parameters with the local override function when someone walks through this
	 *	actor */
	UFUNCTION()
	void UpdateMasterGlobalCombatParameters(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	virtual void BeginPlay() override;

};
