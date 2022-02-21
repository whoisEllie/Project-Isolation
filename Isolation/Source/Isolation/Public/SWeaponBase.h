// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCharacter.h"
#include "SWeaponPickup.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "SWeaponBase.generated.h"

class USkeletalMeshComponent;
class USkeletalMesh;
class UStaticMesh;
class UAnimMontage;
class UBlendSpace;
class UNiagaraSystem;
class USoundCue;
class UPhysicalMaterial;
class UDataTable;
class ASWeaponPickup;

UENUM()
enum class EAttachmentType : uint8
{
	Barrel		UMETA(DisplayName = "Barrel Attachment"),
	Magazine	UMETA(DisplayName = "Magazine Attachment"),
	Sights		UMETA(DisplayName = "Sights Attachment"),
	Stock		UMETA(DispayName = "Stock Attachment"),
	Grip		UMETA(DispayName = "Grip Attachment"),
};

USTRUCT(BlueprintType)
struct FAttachmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	USkeletalMesh* AttachmentMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UStaticMesh* PickupMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	EAttachmentType AttachmentType;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	FName MuzzleLocationOverride;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	FName ParticleSpawnLocationOverride;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	bool bSilenced;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	USoundBase* FiringSoundOverride;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	USoundBase* SilencedFiringSoundOverride;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float BaseDamageImpact;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float WeaponPitchVariationImpact;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float WeaponYawVariationImpact;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UBlendSpace* BS_Walk;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UBlendSpace* BS_ADS_Walk;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UAnimSequence* Anim_Idle;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UAnimSequence* Anim_ADS_Idle;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UAnimSequence* Anim_Sprint;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	EAmmoType AmmoToUse;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	int ClipCapacity;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	int ClipSize;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float WeaponHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float FireRate;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	bool AutomaticFire;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float VerticalCameraOffset;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UCurveFloat* VerticalRecoilCurve;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UCurveFloat* HorizontalRecoilCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	TSubclassOf<UCameraShakeBase> RecoilCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float VerticalRecoilMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	float HorizontalRecoilMultiplier;
};


USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	//Pickup reference
	UPROPERTY(EditDefaultsOnly, Category = "General")
	TSubclassOf<ASWeaponPickup> PickupReference;

	// Determines whether the weapon is automatic or not
	UPROPERTY(EditDefaultsOnly, Category = "General")
	bool bAutomaticFire;
	// Rate of fire (in time between shots) for the weapon
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float RateOfFire;
	// Determines if the weapon can have a round in the chamber or not
	UPROPERTY(EditDefaultsOnly, Category = "General")
	bool bCanBeChambered;
	// We wait for the animation to finish before the player is allowed to fire again (for weapons where the character has to perform an action before being able to fire again) Requires fireMontage to be set
	UPROPERTY(EditDefaultsOnly, Category = "Animation Variables")
	bool bWaitForAnim;
	// Determines the socket or bone with which the weapon will be attached to the character's hand (typically the root bone or the grip bone)
	UPROPERTY(EditDefaultsOnly, Category = "General")
	FName WeaponAttachmentSocketName;
	// Determines if the weapon should fire more than one 'pellet' every time it is fired. Enable if you'd like your weapon to act like a shotgun
	UPROPERTY(EditDefaultsOnly, Category = "General")
	bool bIsShotgun;
	// Determines if the weapon should use a silenced sound effect
	UPROPERTY(EditDefaultsOnly, Category = "General")
	bool bIsSilenced;
	// Determines the amount of pellets a shotgun will fire
	UPROPERTY(EditDefaultsOnly, Category = "General")
	int ShotgunPelletCount;
	// The name of the socket on the meshComp where the line trace is shot from (start point)
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	FName MuzzleSocketName;
	// The name of the socket on the meshComp where the muzzle flash effect is played from
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	FName ParticleSocketName;
	// The distance the shot will travel
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	float LengthMultiplier;
	// The pitch variation applied to the bullet as it leaves the barrel
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	float WeaponPitchVariation;
	// The yaw variation applied to the bullet as it leaves the barrel
	UPROPERTY(EditDefaultsOnly, Category = "Line Trace")
	float WeaponYawVariation;
	// unmodified damage values of this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float BaseDamage;
	// multiplier to be applied when the player hits an enemy's head bone
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float HeadshotMultiplier;

	// Damage surfaces

	// surface (physical material) for areas which should spawn blood particles when hit and receive normal damage (equivalent to the baseDamage variable)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* NormalDamageSurface;
	// surface (physical material) for areas which should spawn blood particles when hit and receive boosted damage (equivalent to the baseDamage variable multiplied by the headshotMultiplier)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* HeadshotDamageSurface;
	// surface (physical material) for areas which should spawn ground particles when hit)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* GroundSurface;
	// surface (physical material) for areas which should spawn rock particles when hit)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Surfaces")
	UPhysicalMaterial* RockSurface;

	// VFX
	
	// particle effect (Niagara system) to be spawned when an enemy is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* EnemyHitEffect;
	// particle effect (Niagara system) to be spawned when the ground is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* GroundHitEffect;
	// particle effect (Niagara system) to be spawned when a rock is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* RockHitEffect;
	// particle effect (Niagara system) to be spawned when no defined type is hit
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* DefaultHitEffect;

	// particle effect (Niagara system) to be spawned at the muzzle when a shot is fired
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* MuzzleFlash;

	// Animation Montages

	// The animation montage played every time a bullet is fired
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* FireMontage;
	// The animation montage played every time the character reloads with ammunition still present in the magazine
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* ReloadMontage;
	// The animation montage played every time the character reloads with a completely empty clip
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* EmptyReloadMontage;

	// Sound bases

	// Firing sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases	")
	USoundBase* FireSound;
	// Silenced firing sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases	")
	USoundBase* SilencedSound;
	// Empty firing sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases	")
	USoundBase* EmptyFireSound;
	
	// Weapon Degradation
	
	// The amount of health taken away from the weapon every time the trigger is pulled
	UPROPERTY(EditDefaultsOnly, Category = "Degradation")
	float WeaponDegradationRate;
	
};

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

	// Applies recoil to the player controller
	void Recoil() const;
	
	// Plays the reload animation and sets a timer based on the length of the reload montage
	void Reload();

	// Updates ammunition values (we do this after the animation has finished for cleaner UI updates and to prevent the player from being able to switch weapons to skip the reload animation)
	void UpdateAmmo();

	// Allows the player to fire again
	void EnableFire();
	
	void SpawnAttachments(TArray<FName> AttachmentsArray);

	// Data table reference
	UPROPERTY(EditDefaultsOnly, Category = "Data Table")
	UDataTable* WeaponDataTable;

	FWeaponData* WeaponData;

	UPROPERTY(EditDefaultsOnly, Category = "Data Table")
	UDataTable* AttachmentsDataTable;

	FAttachmentData* AttachmentData;
	
	// Attachments
	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	TArray<FName> AttachmentNameArray;
	
	// Debug boolean, toggle for debug strings and line traces to be shown
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowDebug;

	

	// Components
	
	// The main skeletal mesh - holds the weapon model
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	// The skeletal mesh used to hold the current barrel attachment
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* BarrelAttachment;

	// The skeletal mesh used to hold the current magazine attachment
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* MagazineAttachment;

	// The skeletal mesh used to hold the current sights attachment
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* SightsAttachment;

	// The skeletal mesh used to hold the current stock attachment
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* StockAttachment;

	// The skeletal mesh used to hold the current grip attachment
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* GripAttachment;
	
	// General

	// Determines if the player can fire
	bool bCanFire;
	// Keeps track of whether the weapon is being reloaded
	bool bIsReloading;
	// Is the weapon silenced or not (determines sounds)
	bool bSilenced;

	float DamageModifier;

	float WeaponPitchModifier;

	float WeaponYawModifier;

	// Sound Cues
	UPROPERTY(BlueprintReadOnly, Category = "Sounds")
	USoundBase* SoundOverride;

	UPROPERTY(BlueprintReadOnly, Category = "Sounds")
	USoundBase* SilencedOverride;

	// Line Trace

	// The override for the weapon socket, in the case that we have a barrel attachment
	FName SocketOverride;
	// The override for the particle system socket, in the case that we have a barrel attachment
	FName ParticleSocketOverride;
	// Keeps track of the starting position of the line trace
	FVector TraceStart;
	// keeps track of the starting rotation of the line trace (required for calculating the trace end point)
	FRotator TraceStartRotation;
	// keeps track of the vector direction of the line trace (derived from rotation)
	FVector TraceDirection;
	// end point of the line trace
	FVector TraceEnd;
	// The range of the weapon


	// collision parameters for spawning the line trace
	FCollisionQueryParams QueryParams;

	// Damage

	// hit result variable set when a line trace is spawned
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FHitResult Hit;
	// internal variable used to keep track of the final damage value after modifications
	float FinalDamage;
	// damage type (set in blueprints)
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;
	

	// Animation

	// value used to keep track of the length of animations for timers
	float AnimTime;
	// Blend space
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UBlendSpace* WalkBlendSpace;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
	UBlendSpace* ADSWalkBlendSpace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
	UAnimSequence* Anim_Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
	UAnimSequence* Anim_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
	UAnimSequence* Anim_ADS_Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
	float VerticalCameraOffset;


	// Timers
	
	// The timer that handles automatic fire
	FTimerHandle ShotDelay;
	// The timer that is used when we need to wait for an animation to finish before being able to fire again
	FTimerHandle AnimationWaitDelay;
	// The timer used to keep track of how long a reloading animation takes and only assigning variables 
	FTimerHandle ReloadingDelay;

	// Recoil

	void StartRecoil();
	
	FTimeline VerticalRecoilTimeline;
	UFUNCTION()
	void HandleVerticalRecoilProgress(float value) const;
	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	UCurveFloat* VerticalRecoilCurve;

	FTimeline HorizontalRecoilTimeline;
	UFUNCTION()
	void HandleHorizontalRecoilProgress(float value) const;
	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	UCurveFloat* HorizontalRecoilCurve;

	FTimeline RecoilRecoveryTimeline;
	UFUNCTION()
	void HandleRecoveryProgress(float value) const;
	UPROPERTY(EditDefaultsOnly, Category = "Curves")
	UCurveFloat* RecoveryCurve;
	FRotator ControlRotation;

	void RecoilRecovery();
	bool bShouldRecover;

	UPROPERTY()
	TSubclassOf<UCameraShakeBase> RecoilCameraShake;

	float VerticalRecoilModifier;
	float HorizontalRecoilModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Particles")
	UNiagaraSystem* EjectedCasing;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
