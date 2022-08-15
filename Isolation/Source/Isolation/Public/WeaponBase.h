// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class ASWeaponBase;
class USkeletalMeshComponent;
class USkeletalMesh;
class UStaticMesh;
class UAnimMontage;
class UNiagaraSystem;
class UBlendSpace;
class USoundCue;
class UPhysicalMaterial;
class UDataTable;
class ASWeaponPickup;

/** Enumerator holding the 4 types of ammunition that weapons can use (used as part of the FSingleWeaponParams struct)
 * and to keep track of the total ammo the player has (ammoMap) */
UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Pistol       UMETA(DisplayName = "Pistol Ammo"),
	Rifle        UMETA(DisplayName = "Rifle Ammo"),
	Shotgun      UMETA(DisplayName = "Shotgun Ammo"),
	Special		 UMETA(DisplayName = "Special Ammo"),
};

UENUM()
enum class EAttachmentType : uint8
{
	Barrel		UMETA(DisplayName = "Barrel Attachment"),
	Magazine	UMETA(DisplayName = "Magazine Attachment"),
	Sights		UMETA(DisplayName = "Sights Attachment"),
	Stock		UMETA(DispayName = "Stock Attachment"),
	Grip		UMETA(DispayName = "Grip Attachment"),
};

/** Struct keeping track of important weapon variables modified at runtime */
USTRUCT(BlueprintType)
struct FRuntimeWeaponData
{
	GENERATED_BODY()

	/** A reference to the weapon class of the given weapon */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Data")
	TSubclassOf<ASWeaponBase> WeaponClassReference;

	/** The maximum size of the player's magazine */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Data")
	int ClipCapacity; 

	/** The amount of ammunition currently in the magazine */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Data")
	int ClipSize;

	/** Enumerator holding the 4 possible ammo types defined above */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Data")
	EAmmoType AmmoType;

	/** The current health of the weapon (degradation values are in the weapon class) */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Data")
	float WeaponHealth;

	/** The attachments used in the current weapon */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Data")
	TArray<FName> WeaponAttachments;
};

USTRUCT(BlueprintType)
struct FAttachmentData : public FTableRowBase
{
	GENERATED_BODY()

	// The skeletal mesh displayed on the weapon itself
	UPROPERTY(EditDefaultsOnly, Category = "General")
	USkeletalMesh* AttachmentMesh;

	// The static mesh displayed on the weapon pickup
	UPROPERTY(EditDefaultsOnly, Category = "General")
	UStaticMesh* PickupMesh;

	// The type of attachment
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General")
	EAttachmentType AttachmentType;

	// Attachments that are incompatible with the given attachment
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General")
	TArray<FName> IncompatibleAttachments;

	// The name of the socket with which to override the muzzle 
	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	FName MuzzleLocationOverride;

	// The name of the socket at which to spawn particles for muzzle flash
	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	FName ParticleSpawnLocationOverride;

	// Whether the current barrel attachment is silenced or not
	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	bool bSilenced;

	// The firing sound to use instead of the default for this particular magazine attachment 
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	USoundBase* FiringSoundOverride;

	// The silenced firing sound to use instead of the default for this particular magazine attachment
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	USoundBase* SilencedFiringSoundOverride;

	// The impact that this magazine has on the base damage of the weapon
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float BaseDamageImpact;

	// The pitch variation impact of this attachment
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float WeaponPitchVariationImpact;

	// The yaw variation impact of this attachment
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float WeaponYawVariationImpact;

	// An override for the default walk BlendSpace
	UPROPERTY(EditDefaultsOnly, Category = "Grip")
	UBlendSpace* BS_Walk;

	// An override for the default ADS walk BlendSpace
	UPROPERTY(EditDefaultsOnly, Category = "Grip")
	UBlendSpace* BS_Ads_Walk;

	// An override for the default idle animation sequence
	UPROPERTY(EditDefaultsOnly, Category = "Grip")
	UAnimSequence* Anim_Idle;

	// An override for the default ADS idle animation sequence
	UPROPERTY(EditDefaultsOnly, Category = "Grip")
	UAnimSequence* Anim_Ads_Idle;

	// An override for the default sprint animation sequence
	UPROPERTY(EditDefaultsOnly, Category = "Grip")
	UAnimSequence* Anim_Sprint;

	// The shooting animation for the weapon itself (bolt shooting back/forward)
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimSequence* Gun_Shot;

	// The ammunition type to be used (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	EAmmoType AmmoToUse;

	// The clip capacity of the weapon (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	int ClipCapacity;

	// The clip size of the weapon (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	int ClipSize;

	// The default health of the weapon (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float WeaponHealth = 100.0f;

	// The rate of fire of this magazine attachment
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	float FireRate;

	// Whether this magazine supports automatic fire
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	bool AutomaticFire;

	// The offset applied to the camera to align with the sights
	UPROPERTY(EditDefaultsOnly, Category = "Sights")
	float VerticalCameraOffset;

	// Whether the player's FOV should change when aiming with this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Sights")
	bool bAimingFOV = false;

	// The decrease in FOV of the camera to when aim down sights
	UPROPERTY(EditDefaultsOnly, Category = "Sights")
	float AimingFOVChange;

	// Whether this weapon has a scope and we need to render a SceneCaptureComponent2D
	UPROPERTY(EditDefaultsOnly, Category = "Sights")
	bool bIsScope = false;

	// The Magnification of the scope
	UPROPERTY(EditDefaultsOnly, Category = "Sights")
	float ScopeMagnification = 1.0f;

	// The linear FOV at a magnification of 1x
	UPROPERTY(EditDefaultsOnly, Category = "Sights")
	float UnmagnifiedLFoV = 200.0f;
	
	// The vertical recoil curve to be used with this magazine
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	UCurveFloat* VerticalRecoilCurve;

	// The horizontal recoil curve to be used with this magazine
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	UCurveFloat* HorizontalRecoilCurve;

	// The camera shake to be applied to the recoil from this magazine
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	TSubclassOf<UCameraShakeBase> RecoilCameraShake;

	// How much this attachment multiplies the vertical recoil of the weapon
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float VerticalRecoilMultiplier;

	// How much this attachment multiplies the horizontal recoil of this weapon
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float HorizontalRecoilMultiplier;

	// Whether this magazine fires shotgun shells (should we fire lots of pellets or just one bullet?)
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	bool bIsShotgun = false;

	// The range of the shotgun shells in this magazine
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	float ShotgunRange;

	// The amount of pellets fired
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	int ShotgunPellets;

	// The increase in shot variation when the player is not aiming down the sights
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	float AccuracyDebuff = 1.25f;

	// An override for the weapon's empty reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	UAnimationAsset* EmptyWeaponReload;

	// An override for the weapon's reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	UAnimationAsset* WeaponReload;

	// An override for the player's empty reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	UAnimMontage* EmptyPlayerReload;

	// An override for the player's reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Magazine")
	UAnimMontage* PlayerReload;

	// Unequip animation for the current weapon
	UPROPERTY(EditDefaultsOnly, Category = "Grip")
	UAnimMontage* WeaponEquip;
	
};


USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	// Required

	//Pickup reference
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	TSubclassOf<ASWeaponPickup> PickupReference;
	
	// Determines the socket or bone with which the weapon will be attached to the character's hand (typically the root bone or the grip bone)
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	FName WeaponAttachmentSocketName;
		
	// The distance the shot will travel
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float LengthMultiplier;
		
	// unmodified damage values of this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float BaseDamage;
	
	// multiplier to be applied when the player hits an enemy's head bone
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float HeadshotMultiplier;

	// The amount of health taken away from the weapon every time the trigger is pulled
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float WeaponDegradationRate;

	// The pitch variation applied to the bullet as it leaves the barrel
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float WeaponPitchVariation;
	
	// The yaw variation applied to the bullet as it leaves the barrel
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float WeaponYawVariation;

	// Attachments

	// Whether this weapon has a unique set of attachments and is broken up into multiple meshes or is unique
	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	bool bHasAttachments = true;

	// The table which holds the attachment data
	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	UDataTable* AttachmentsDataTable;

	// Animations

	// The walking BlendSpace
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UBlendSpace* BS_Walk;

	// The ADS Walking BlendSpace
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UBlendSpace* BS_Ads_Walk;

	// The Idle animation sequence
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimSequence* Anim_Idle;

	// The ADS Idle animation sequence
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimSequence* Anim_Ads_Idle;

	// The weapon's empty reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimationAsset* EmptyWeaponReload;

	// The weapon's reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimationAsset* WeaponReload;

	// The player's empty reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimMontage* EmptyPlayerReload;

	// The player's reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimMontage* PlayerReload;

	// The sprinting animation sequence
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimSequence* Anim_Sprint;

	// The shooting animation for the weapon itself (bolt shooting back/forward)
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimSequence* Gun_Shot;
	
	// An override for the player's reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimMontage* WeaponEquip;

	// An override for the player's reload animation
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UAnimMontage* WeaponUnequip;

	// Firing Mechanisms

	// Determines if the weapon can have a round in the chamber or not
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bCanBeChambered;

	// Whether the weapon is silenced or not
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bSilenced;

	// We wait for the animation to finish before the player is allowed to fire again (for weapons where the character has to perform an action before being able to fire again) Requires fireMontage to be set
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bWaitForAnim;

	// Whether this weapon is a shotgun or not
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bIsShotgun = false;

	// Whether the player's FOV should change when aiming with this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bAimingFOV = false;

	// The decrease in FOV of the camera to when aim down sights
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	float AimingFOVChange;

	// Whether this weapon has a scope and we need to render a SceneCaptureComponent2D
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bIsScope = false;

	// The Magnification of the scope
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	float ScopeMagnification = 1.0f;

	// The linear FOV at a magnification of 1x
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	float UnmagnifiedLFoV = 200.0f;
	
	// The name of the socket which denotes the end of the muzzle (used for gunfire)
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	FName MuzzleLocation;

	// The name of the socket at which to spawn particles for muzzle flash
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	FName ParticleSpawnLocation;

	// The ammunition type to be used (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	EAmmoType AmmoToUse;

	// The clip capacity of the weapon (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	int ClipCapacity;

	// The clip size of the weapon (Spawned on the pickup)
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	int ClipSize;

	// The rate of fire of the weapon
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	float RateOfFire;

	// Whether this weapon supports automatic fire
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	bool bAutomaticFire;
		
	// The vertical recoil curve to be used with this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UCurveFloat* VerticalRecoilCurve;

	// The horizontal recoil curve to be used with this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	UCurveFloat* HorizontalRecoilCurve;

	// The camera shake to be applied to the recoil from this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	TSubclassOf<UCameraShakeBase> RecoilCameraShake;
	
	// The range of the shotgun shells of this weapon
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	float ShotgunRange;

	// The amount of pellets fired
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	int ShotgunPellets;

	// The increase in shot variation when the player is not aiming down the sights
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)")
	float AccuracyDebuff = 1.25f;

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

	// particle effect to be spawned at the muzzle when a shot is fired
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UParticleSystem* MuzzleFlash;

	// particle effect to be spawned at the muzzle that shows the path of the bullet
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UParticleSystem* BulletTrace;

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
	void Recoil();
	
	// Plays the reload animation and sets a timer based on the length of the reload montage
	void Reload();

	// Updates ammunition values (we do this after the animation has finished for cleaner UI updates and to prevent the player from being able to switch weapons to skip the reload animation)
	void UpdateAmmo();

	// Allows the player to fire again
	void EnableFire();

	// Spawns the weapons attachments and applies their data/modifications to the weapon's statistics 
	void SpawnAttachments();

	// Begins applying recoil to the weapon
	void StartRecoil();

	// Initiates the recoil function
	void RecoilRecovery();

	// Interpolates the player back to their initial view vector
	UFUNCTION()
	void HandleRecoveryProgress(float value) const;

	float FOVFromMagnification() const;

	void RenderScope() const;

	UFUNCTION(BlueprintCallable)
	void SetShowDebug(bool IsVisible)
	{
		bShowDebug = IsVisible;
	};

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	float ScopeFrameRate = 60.0f;

	// Data table reference
	UPROPERTY(EditDefaultsOnly, Category = "Data Table")
	UDataTable* WeaponDataTable;

	// The Key reference to the weapon data table
	UPROPERTY(EditDefaultsOnly, Category = "Data Table")
	FString DataTableNameRef;

	// Reference to the data stored in the weapon DataTable
	FWeaponData WeaponData;

	// Reference to the data stored in the attachment DataTable
	FAttachmentData* AttachmentData;
	
	// Attachments
	TArray<FName> AttachmentNameArray;
	
	// Debug boolean, toggle for debug strings and line traces to be shown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

	FRuntimeWeaponData GeneralWeaponData;

	// Components
	
	// The main skeletal mesh - holds the weapon model
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	// The skeletal mesh used to hold the current barrel attachment
	UPROPERTY()
	USkeletalMeshComponent* BarrelAttachment;

	// The skeletal mesh used to hold the current magazine attachment
	UPROPERTY()
	USkeletalMeshComponent* MagazineAttachment;

	// The skeletal mesh used to hold the current sights attachment
	UPROPERTY()
	USkeletalMeshComponent* SightsAttachment;

	// The skeletal mesh used to hold the current stock attachment
	UPROPERTY()
	USkeletalMeshComponent* StockAttachment;

	// The skeletal mesh used to hold the current grip attachment
	UPROPERTY()
	USkeletalMeshComponent* GripAttachment;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USceneCaptureComponent2D* ScopeCaptureComponent;
	
	// General

	// Determines if the player can fire
	bool bCanFire = true;
	
	// Keeps track of whether the weapon is being reloaded
	bool bIsReloading;

	// The sum of the modifications the attachments make to damage
	float DamageModifier;

	// The sum of the modifications the attachments make to pitch
	float WeaponPitchModifier;

	// The sum of the modifications the attachments make to yaw
	float WeaponYawModifier;
	

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
	
	// collision parameters for spawning the line trace
	FCollisionQueryParams QueryParams;

	// Damage

	// hit result variable set when a line trace is spawned
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FHitResult Hit;
	
	// damage type (set in blueprints)
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;

	// internal variable used to keep track of the final damage value after modifications
	float FinalDamage;
	
	
	// Timers
	
	// The timer that handles automatic fire
	FTimerHandle ShotDelay;
	// The timer that is used when we need to wait for an animation to finish before being able to fire again
	FTimerHandle AnimationWaitDelay;
	// The timer used to keep track of how long a reloading animation takes and only assigning variables 
	FTimerHandle ReloadingDelay;
	// The timer used for rendering the scope image
	FTimerHandle ScopeRenderTimer;

	// Recoil



	// The curve for vertical recoil (set from WeaponData)
	UPROPERTY()
	UCurveFloat* VerticalRecoilCurve;

	// The timeline for vertical recoil (generated from the curve)
	FTimeline VerticalRecoilTimeline;

	// The curve for horizontal recoil (set from WeaponData)
	UPROPERTY()
	UCurveFloat* HorizontalRecoilCurve;

	// The timeline for horizontal recoil (generated from the curve)
	FTimeline HorizontalRecoilTimeline;

	// The curve for recovery 
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	UCurveFloat* RecoveryCurve;

	// The timeline for recover (set from the curve)
	FTimeline RecoilRecoveryTimeline;
	
	// A value to temporarily cache the player's control rotation so that we can return to it
	FRotator ControlRotation;

	// Keeping track of whether we should do a recoil recovery after finishing firing or not
	bool bShouldRecover;

	// Used in recoil to make sure the first shot has properly applied recoil
	int ShotsFired;
	
	// The base multiplier for vertical recoil, modified by attachments
	float VerticalRecoilModifier;

	// The base multiplier for horizontal recoil, modified by attachments
	float HorizontalRecoilModifier;

	// The ejected casing particle effect to be played after each shot
	UPROPERTY(EditDefaultsOnly, Category = "Particles")
	UNiagaraSystem* EjectedCasing;
	
	// Animation

	// Value used to keep track of the length of animations for timers
	float AnimTime;

	// The offset given to the camera in order to align the gun sights
	UPROPERTY(BlueprintReadOnly, Category = "Attachments")
	float VerticalCameraOffset;
	
	// Local instances of animations for use in AnimBP (Set from WeaponData and/or Attachments)

	UPROPERTY()
	UAnimMontage* WeaponEquip;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animations")
	UBlendSpace* WalkBlendSpace;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attachments")
	UBlendSpace* ADSWalkBlendSpace;

	UPROPERTY(BlueprintReadOnly, Category = "Attachments")
	UAnimSequence* Anim_Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Attachments")
	UAnimSequence* Anim_Sprint;

	UPROPERTY(BlueprintReadOnly, Category = "Attachments")
	UAnimSequence* Anim_ADS_Idle;

	UPROPERTY()
	UAnimationAsset* EmptyWeaponReload;
	
	UPROPERTY()
	UAnimationAsset* WeaponReload;
	
	UPROPERTY()
	UAnimMontage* EmptyPlayerReload;
	
	UPROPERTY()
	UAnimMontage* PlayerReload;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
