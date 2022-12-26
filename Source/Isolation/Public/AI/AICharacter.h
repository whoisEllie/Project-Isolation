// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "FPSCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "AICharacter.generated.h"

/**
 * 
 */
UCLASS()
class ISOLATION_API AAICharacter : public AFPSCharacter
{
	GENERATED_BODY()

public:

	AAICharacter();

	virtual void BeginPlay() override;

	void UpdateWeapon(const TSubclassOf<AWeaponBase> NewWeapon);

	UFUNCTION(BlueprintCallable)
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable, Category = "AI Character")
	void StartFire(int ShotsToTake);

	UFUNCTION(BlueprintCallable, Category = "AI Character")
	void StopFire();

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "AI | Weapon")
	TSubclassOf<AWeaponBase> StarterWeapon;
	
	UPROPERTY()
	AWeaponBase* CurrentWeapon;
};
