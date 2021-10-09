// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SIsolationGameInstance.h"
#include "SWeaponBase.generated.h"

class USkeletalMeshComponent;
class UAnimMontage;
class UNiagaraSystem;
class USoundCue;
class UPhysicalMaterial;

UCLASS()
class ISOLATION_API ASWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeaponBase();
	
	// Starts firing the gun (sets the timer for automatic fire)
	void StartFire();
	
	// Stops the timer that allows for automatic fire
	void StopFire();
	
	// Spawns the line trace that deals damage and applies sound/visual effects
	void Fire();
	
	// Plays the reload animation and sets a timer based on the length of the reload montage
	void Reload();

	// Updates ammunition values (we do this after the animation has finished for cleaner UI updates and to prevent the player from being able to switch weapons to skip the reload animation)
	void UpdateAmmo();

	// Allows the player to fire again
	void EnableFire();
	
	// Components
	
	// The main skeletal mesh - holds the weapon model
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* meshComp;
	
	// General

	// Determines if the player can fire
	bool bCanFire;
	// Determines whether the weapon is automatic or not
	UPROPERTY(EditDefaultsOnly, Category = "General")
	bool bAutomaticFire;
	// Keeps track of whether the weapon is being reloaded
	bool bIsReloading;
	// Rate of fire (in time between shots) for the weapon
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float rateOfFire;
	// Determines if the weapon can have a round in the chamber or not
	UPROPERTY(EditDefaultsOnly, Category = "General")
	bool bCanBeChambered;
	// Determines the socket or bone with which the weapon will be attached to the character's hand (typically the root bone or the grip bone)
	UPROPERTY(EditDefaultsOnly, Category = "General")
	FName weaponAttachmentSocketName;

	// Line Trace

	// The name of the socket on the meshComp where the line trace is shot from (start point)
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	FName muzzleSocketName;
	// Keeps track of the starting position of the line trace
	FVector traceStart;
	// keeps track of the starting rotation of the line trace (required for calculating the trace end point)
	FRotator traceStartRotation;
	// keeps track of the vector direction of the line trace (derived from rotation)
	FVector traceDirection;
	// end point of the line trace
	FVector traceEnd;
	// The range of the weapon
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	float lengthMultiplier;

	// collision parameters for spawning the line trace
	FCollisionQueryParams queryParams;

	// Damage

	// hit result variable set when a line trace is spawned
	FHitResult hit;
	// unmodified damage values of this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float baseDamage;
	// multiplier to be applied when the player hits an enemy's head bone
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float headshotMultiplier;
	// internal variable used to keep track of the final damage value after modifications
	float finalDamage;
	// damage type (set in blueprints)
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UDamageType> damageType;

	// Damage surfaces

	// surface (physical material) for areas which should spawn blood particles when hit and receive normal damage (equivalent to the baseDamage variable)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* normalDamageSurface;
	// surface (physical material) for areas which should spawn blood particles when hit and receive boosted damage (equivalent to the baseDamage variable multiplied by the headshotMultiplier)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* headshotDamageSurface;
	// surface (physical material) for areas which should spawn ground particles when hit)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* groundSurface;
	// surface (physical material) for areas which should spawn rock particles when hit)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* rockSurface;

	// VFX
	
	// particle effect (Niagara system) to be spawned when an enemy is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* enemyHitEffect;
	// particle effect (Niagara system) to be spawned when the ground is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* groundHitEffect;
	// particle effect (Niagara system) to be spawned when a rock is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* rockHitEffect;
	// particle effect (Niagara system) to be spawned when no defined type is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* defaultHitEffect;

	// Animation Variables

	// value used to keep track of the length of animations for timers
	float animTime;
	// We wait for the animation to finish before the player is allowed to fire again (for weapons where the character has to perform an action before being able to fire again) Requires fireMontage to be set
	UPROPERTY(EditDefaultsOnly, Category = "Animation Variables")
	bool bWaitForAnim;
	
	// Animation Montages

	// The animation montage played every time a bullet is fired
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* fireMontage;
	// The animation montage played every time the character reloads with ammunition still present in the magazine
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* reloadMontage;
	// The animation montage played every time the character reloads with a completely empty clip
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* emptyReloadMontage;

	// Weapon Degradation
	
	// The amount of health taken away from the weapon every time the trigger is pulled
	UPROPERTY(EditDefaultsOnly, Category = "Degradation")
	float weaponDegredationRate;

	// Timers
	
	// The timer that handles automatic fire
	FTimerHandle shotDelay;
	// The timer that is used when we need to wait for an animation to finish before being able to fire again
	FTimerHandle animationWaitDelay;
	// The timer used to keep track of how long a reloading animation takes and only assigning variables 
	FTimerHandle reloadingDelay;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
