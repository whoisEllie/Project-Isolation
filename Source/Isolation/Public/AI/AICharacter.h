// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseAIWeapon.h"
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

	void UpdateWeapon(const TSubclassOf<ABaseAIWeapon> NewWeapon) const;

	UFUNCTION(BlueprintCallable, Category = "AI Character")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "AI Character")
	void StopFire();
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "AI | Weapon")
	TSubclassOf<ABaseAIWeapon> StarterWeapon;
	
	UPROPERTY()
	ABaseAIWeapon* CurrentWeapon;
};
