// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ASWeaponBase;
class UAnimMontage;
class UCurveFloat;

// Enumerator holding the 4 types of ammunition that weapons can use (used as part of the FsingleWeaponParams struct)
// and to keep track of the total ammo the player has (ammoMap)
UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Pistol       UMETA(DisplayName = "Pistol Ammo"),
	Rifle        UMETA(DisplayName = "Rifle Ammo"),
	Shotgun      UMETA(DisplayName = "Shotgun Ammo"),
	Special		 UMETA(DisplayName = "Special Ammo"),
};

UENUM(BlueprintType)
enum EMovementState
{
	State_Walk      UMETA(DisplayName = "Walking"),
	State_Sprint    UMETA(DisplayName = "Sprinting"),
	State_Crouch    UMETA(DisplayName = "Crouching"),
	State_Slide		UMETA(DisplayName = "Sliding"),
	State_Vault	    UMETA(DisplayName = "Vaulting")
};

USTRUCT(BlueprintType)
struct FWeaponDataStruct
{
	GENERATED_BODY()

	// The maximum size of the player's magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int ClipCapacity; 

	// The amount of ammunition currently in the magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int ClipSize;

	// Enumerator holding the 4 possible ammo types defined above
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Type")
	EAmmoType AmmoType;

	// The current health of the weapon (degradation values are in the weapon class)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float WeaponHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	TArray<FName> WeaponAttachments;
};

UCLASS()
class ISOLATION_API ASCharacter : public ACharacter	
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

	// Switches to new weapon
	void UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, bool bSpawnPickup, FWeaponDataStruct OldDataStruct, bool bStatic, FTransform PickupTransform);

	bool bIsPrimary;

	bool bNewPrimarySpawn;
	bool bNewSecondarySpawn;
	
    //Hands mesh, assignable through blueprints
    UPROPERTY(EditDefaultsOnly, Category = "Components")
    USkeletalMeshComponent* HandsMeshComp;
	//Camera Comp - component for the FPS camera
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UCameraComponent* CameraComp;
	//Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation'
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	// Weapon classes
	// A reference to the player's current primary weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASWeaponBase> PrimaryWeapon;
	// A reference to the player's current secondary weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASWeaponBase> SecondaryWeapon;
	// The player's currently equipped weapon
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	ASWeaponBase* CurrentWeapon;


	// Booleans
	
	// The boolean holding whether the player wants to aim or not
	bool bWantsToAim;
	// The boolean holding whether we are aiming or not
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	bool bIsAiming;
	// The boolean keeping track of whether we're vaulting or not
	bool bIsVaulting;
	// Keeps track whether the player is holding the Crouch button
	bool bHoldingCrouch;
	// Have we performed a slide yet?
	bool bPerformedSlide;
	// Is holding the sprint key
	bool bHoldingSprint;
	// wants to slide? (is holding the crouch/slide key, but not on the ground)
	bool bWantsToSlide;
	// prints debug variables if enabled
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug;
	// keeps track of whether we're sprinting (for animations)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;
	// keeps track of whether we're crouching (for animations)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;
	// keeps track of whether the object we are looking at is one we are able to interact with (used for UI)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	bool bCanInteract;

	// Enumerators

	// Enumerator holding the 4 possible movement states defined above
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement State")
    TEnumAsByte<EMovementState> MovementState; // DONT USE TENUMASBYTE

	// Other
	// Name of the socket we attach our camera to
	UPROPERTY(EditDefaultsOnly, Category = "Other")
	FName CameraSocketName;

	// Weapon variables

	UPROPERTY(EditDefaultsOnly)
	FWeaponDataStruct PrimaryWeaponCacheMap;

	UPROPERTY(EditDefaultsOnly)
	FWeaponDataStruct SecondaryWeaponCacheMap;
	
    
protected:


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Moving forward/backwards (takes axis as input from CharacterMovementComponent.h)
	void MoveForward(float value);

    // Moving left/right (takes axis as input from CharacterMovementComponent.h)
	void MoveRight(float value);

    // Looking up/down (takes axis as input from CharacterMovementComponent.h)
	void LookUp(float value);

    // Looking left/right (takes axis as input from CharacterMovementComponent.h)
	void LookRight(float value);

	// Alternative to the built in Crouch function
	void StartCrouch();

	// Alternative to the built in UnCrouch function
	void StopCrouch();

	// We do this so that we can perform checks based on the height above the player (can they even stand up?) and to handle sliding
	void EndCrouch(bool bToSprint);

	// Starting to sprint
	void StartSprint();

	// Stopping to sprint
	void StopSprint();

	// Starting to slide
	void StartSlide();

	// Stopping to slide
	void StopSlide();

	// Function that runs on tick and checks if we should execute the Vault() functions
	void CheckVault();
	
	// Function that actually executes the Vault
	void Vault(float Height, FTransform TargetTransform);
	
	// Global system to update movement speed
	void UpdateMovementSpeed();

	void SwapToPrimary();

	void SwapToSecondary();

	// Fires the weapon
	void StartFire();

	// Stops firing the weapon
	void StopFire();

	// Reloads the weapon
	void Reload();

	void StartADS();

	void StopADS();

	void CheckAngle();

	UFUNCTION()
	void TimelineProgress(float value);

	void WorldInteract();

	void InteractionIndicator();

	void ScrollWeapon();

	FTimeline VaultTimeline;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;

	FHitResult Hit;

	FHitResult VaultHit;

	FHitResult AngleHit;

	FHitResult InteractionHit;

	FTransform VaultStartLocation;

	FTransform VaultEndLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* VaultMontage;

	// Floats
	
	// Variables for crouch system
	// Sets the height of the player when crouched
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FinalCapsuleHalfHeight;
	// Set in the default values, the base height of the capsule
	float DefaultCapsuleHalfHeight;
	// Determines the rate at which the character crouches
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float CrouchSpeed;
	// Slide time
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float SlideTime;
	// Default FOV
	float DefaultFOV;
	// target FOV
	float SpeedFOV;
	// change speed for the fov
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FOVChangeSpeed;
	// amount for FOV to increase
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FOVChangeAmount;
	// current angle of floor
	float FloorAngle;
	// vector of floor
	FVector FloorVector;
	// The height of the highest surface that the player can mantle up onto
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float MaxVaultHeight;
	// The forward movement value (used to drive animations)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	float ForwardMovement;
	// The right movement value (used to drive animations)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	float RightMovement;
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float WeaponSpawnDistance;
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float InteractDistance;

	// Integers

	// amount of traces to draw for vault calculations, to get distance in unreal units, multiply by 5
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	int VaultTraceAmount;


	// Variables for movement
	// The maximum speed of the character when in the sprint state
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed;
	// The maximum speed of the character when in the walk state
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed;
	// Determines the speed of the character when crouched
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchMovementSpeed;
	// Determines the speed of the character when sliding
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SlideSpeed;
	//
	FTransform LocalTargetTransform;

	// Timer managers
	FTimerHandle SlideStop;
    
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
