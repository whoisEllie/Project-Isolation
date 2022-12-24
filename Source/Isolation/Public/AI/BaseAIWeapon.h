// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"
#include "BaseAIWeapon.generated.h"

USTRUCT(BlueprintType)
struct FAIWeaponData: public FTableRowBase
{
	GENERATED_BODY()

	/**Pickup reference */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	TSubclassOf<AWeaponPickup> PickupReference;

	/** Determines the socket or bone with which the weapon will be attached to the character's hand (typically the root bone or the grip bone) */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	FName WeaponAttachmentSocketName;
	
	/** The distance the shot will travel */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float LengthMultiplier;
		
	/** unmodified damage values of this weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float BaseDamage;
	
	/** multiplier to be applied when the player hits an enemy's head bone */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float HeadshotMultiplier;

	/** The pitch variation applied to the bullet as it leaves the barrel */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float WeaponPitchVariation;
	
	/** The yaw variation applied to the bullet as it leaves the barrel */
	UPROPERTY(EditDefaultsOnly, Category = "Required")
	float WeaponYawVariation;

	/** Attachments */

	/** Whether this weapon has a unique set of attachments and is broken up into multiple meshes or is unique */
	UPROPERTY(EditDefaultsOnly, Category = "Attachments")
	bool bHasAttachments = true;

	/** The table which holds the attachment data */
	UPROPERTY(EditDefaultsOnly, Category = "Attachments", meta=(EditCondition="bHasAttachments"))
	UDataTable* AttachmentsDataTable;

	/** Animations */

	//TODO: Implement modular animation system for AI weapons

	/** Firing Mechanisms */

	/** Determines if the weapon can have a round in the chamber or not */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bCanBeChambered;

	/** Whether the weapon is silenced or not */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bSilenced;

	/** We wait for the animation to finish before the player is allowed to fire again (for weapons where the character
	 *	has to perform an action before being able to fire again) Requires fireMontage to be set */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bWaitForAnim;

	/** Whether this weapon is a shotgun or not */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bIsShotgun = false;

	/** Whether the player's FOV should change when aiming with this weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bAimingFOV = false;

	/** The decrease in FOV of the camera to when aim down sights */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	float AimingFOVChange;

	/** Whether this weapon has a scope and we need to render a SceneCaptureComponent2D */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bIsScope = false;

	/** The Magnification of the scope */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	float ScopeMagnification = 1.0f;

	/** The linear FOV at a magnification of 1x */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	float UnmagnifiedLFoV = 200.0f;
	
	/** The name of the socket which denotes the end of the muzzle (used for gunfire) */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	FName MuzzleLocation;

	/** The name of the socket at which to spawn particles for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	FName ParticleSpawnLocation;

	/** The ammunition type to be used (Spawned on the pickup) */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	EAmmoType AmmoToUse;

	/** The clip capacity of the weapon (Spawned on the pickup) */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	int ClipCapacity;

	/** The clip size of the weapon (Spawned on the pickup) */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	int ClipSize;

	/** The rate of fire of the weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	float RateOfFire;

	/** Whether this weapon supports automatic fire */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	bool bAutomaticFire;

	/** The range of the shotgun shells of this weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	float ShotgunRange;

	/** The amount of pellets fired */
	UPROPERTY(EditDefaultsOnly, Category = "Unique Weapon (No Attachments)", meta=(EditCondition="!bHasAttachments"))
	int ShotgunPellets;

	/** VFX */
	
	/** particle effect (Niagara system) to be spawned when an enemy is hit */
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(EditCondition="!bHasAttachments"))
	UNiagaraSystem* EnemyHitEffect;
	
	/** particle effect (Niagara system) to be spawned when the ground is hit */
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(EditCondition="!bHasAttachments"))
	UNiagaraSystem* GroundHitEffect;
	
	/** particle effect (Niagara system) to be spawned when a rock is hit */
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(EditCondition="!bHasAttachments"))
	UNiagaraSystem* RockHitEffect;
	
	/** particle effect (Niagara system) to be spawned when no defined type is hit */
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(EditCondition="!bHasAttachments"))
	UNiagaraSystem* DefaultHitEffect;

	/** particle effect to be spawned at the muzzle when a shot is fired */
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(EditCondition="!bHasAttachments"))
	UParticleSystem* MuzzleFlash;

	/** particle effect to be spawned at the muzzle that shows the path of the bullet */
	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta=(EditCondition="!bHasAttachments"))
	UParticleSystem* BulletTrace;

	/** Sound bases */

	/** Firing sound */
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases", meta=(EditCondition="!bHasAttachments"))
	USoundBase* FireSound;
	
	/** Silenced firing sound */
	UPROPERTY(EditDefaultsOnly, Category = "Sound bases", meta=(EditCondition="!bHasAttachments"))
	USoundBase* SilencedSound;
	
};


UCLASS()
class ISOLATION_API ABaseAIWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseAIWeapon();

	void Fire();

	void SpawnAttachments();

	FAIWeaponData* GetAIWeaponData() { return &StaticWeaponData;}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY()
	USkeletalMeshComponent* BarrelAttachment;

	UPROPERTY()
	USkeletalMeshComponent* MagazineAttachment;

	UPROPERTY()
	USkeletalMeshComponent* SightsAttachment;

	UPROPERTY()
	USkeletalMeshComponent* StockAttachment;

	UPROPERTY()
	USkeletalMeshComponent* GripAttachment;

#pragma region USER_VARIABLES

	/** damage type (set in blueprints) */
	UPROPERTY(EditDefaultsOnly, Category = "Data | Damage")
	TSubclassOf<UDamageType> DamageType;

	/** Data table reference */
	UPROPERTY(EditDefaultsOnly, Category = "Data | Data Table")
	UDataTable* WeaponDataTable;

	/** The Key reference to the weapon data table */
	UPROPERTY(EditDefaultsOnly, Category = "Data | Data Table")
	FString DataTableNameRef;

#pragma endregion 

#pragma region INTERNAL_VARIABLES

	bool bCanFire;

	/** The sum of the modifications the attachments make to damage */
	float DamageModifier;

	/** The sum of the modifications the attachments make to pitch */
	float WeaponPitchModifier;

	/** The sum of the modifications the attachments make to yaw */
	float WeaponYawModifier;

	FRuntimeWeaponData RuntimeWeaponData;

	FAIWeaponData StaticWeaponData;

	FAttachmentData* AttachmentData;

	FHitResult Hit;

#pragma endregion 
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
