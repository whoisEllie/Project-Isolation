// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeaponBase.generated.h"

class USkeletalMeshComponent;

UCLASS()
class ISOLATION_API ASWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeaponBase();
	
	void StartFire();
	
	void StopFire();
	
	void Fire();
	
	void Reload();
	
	// Components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* meshComp;
	
	// General

	bool canFire;

	// Line Trace

	FName muzzleSocketName;
	FVector traceStart;
	FRotator traceStartRotation;
	FVector traceDirection;
	FVector traceEnd;

	FCollisionQueryParams queryParams;

	// Damage

	FHitResult hit;
	float baseDamage;
	float headshotMultiplier;
	float finalDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	TSubclassOf<UDamageType> damageType;

	// Damage surfaces

	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	UPhysicalMaterial* NormalDamageSurface;
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	UPhysicalMaterial* HeadshotSurface;
	
	// Weapon Degradation

	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float weaponDegredationRate;

	// Timers

	FTimerHandle timerHandleShotDelay;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
