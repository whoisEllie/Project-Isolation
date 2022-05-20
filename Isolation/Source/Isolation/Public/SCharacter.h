// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SHUDWidget.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ASWeaponBase;
class UAnimMontage;
class UCurveFloat;
class UBlendSpace;

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
enum class EMovementState : uint8
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

	// The attachments used in the current weapon
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

	// Switching to a new weapon
	void UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, bool bSpawnPickup, FWeaponDataStruct OldDataStruct, bool bStatic, FTransform PickupTransform);

	// Functions for UI
	// Get the amount of ammunition currently loaded into the weapon
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponLoadedAmmo() const;

	// Get the amount of ammunition remaining (i.e., the amount of ammo the player has of the ammo type used by the
	// current weapon)
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponRemainingAmmo() const;
		
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

	// Whether we should render a crosshair or not
	UPROPERTY(BlueprintReadWrite, Category = "Crosshair")
	bool bShowCrosshair;
	
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
	
	// keeps track of whether the interaction object is a weapon pickup (used for UI)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	bool bInteractionIsWeapon;

	// Whether the player is holding the primary weapon (or not, and are thus holding the secondary weapon)
	bool bIsPrimary;

	// Enumerators

	// Enumerator holding the 5 possible movement states defined above in EMovementState
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement State")
    EMovementState MovementState;

	// UI

	// The current message to be displayed above the screen (if any)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	FText InteractText;

	// Other
	
	// Name of the socket we attach our camera to
	UPROPERTY(EditDefaultsOnly, Category = "Other")
	FName CameraSocketName;

	// Weapon variables

	// Weapon cache maps
	UPROPERTY(EditDefaultsOnly)
	FWeaponDataStruct PrimaryWeaponCacheMap;

	UPROPERTY(EditDefaultsOnly)
	FWeaponDataStruct SecondaryWeaponCacheMap;

	// Hand animations for when the player has no weapon 
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UBlendSpace* BS_Walk;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UBlendSpace* BS_ADS_Walk;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Idle;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_ADS_Idle;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Sprint;

	UPROPERTY()
	USHUDWidget* UserWidget;
    
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
	void Vault(FTransform TargetTransform);
	
	// Global system to update movement speed
	void UpdateMovementSpeed();

	void SwapToPrimary();

	void SwapToSecondary();
	
	// Changing the weapon with the scroll wheel
	void ScrollWeapon();

	// Fires the weapon
	void StartFire();

	// Stops firing the weapon
	void StopFire();

	// Reloads the weapon
	void Reload();

	// Starts ADS
	void StartADS();

	// Ends ADS
	void StopADS();

	// Checks the angle of the floor to determine slide behaviour
	void CheckAngle();

	// Ticks the timeline
	UFUNCTION()
	void TimelineProgress(float value);

	// Interaction with the world using SInteractInterface
	void WorldInteract();

	// Displaying the indicator for interaction
	void InteractionIndicator();

	// The curve for vaulting
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;
	
	// The timeline for vaulting (generated from the curve)
	FTimeline VaultTimeline;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USHUDWidget> HUDWidget;	
	
	// Hit results for various line traces
	FHitResult Hit;

	FHitResult VaultHit;

	FHitResult AngleHit;

	FHitResult InteractionHit;

	FTransform VaultStartLocation;

	FTransform VaultEndLocation;

	// The montage for the hands vault animation 
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
	float SlideTime = 1.0f;
	
	// Default FOV
	float DefaultFOV;
	
	// target FOV
	float SpeedFOV;
	
	// change speed for the fov
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FOVChangeSpeed = 2.0f;
	
	// amount for FOV to increase
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FOVChangeAmount = 5.0f;
	
	// current angle of floor
	float FloorAngle;
	
	// vector of floor
	FVector FloorVector;
	
	// The height of the highest surface that the player can mantle up onto
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float MaxVaultHeight = 200.0f;
	
	// The forward movement value (used to drive animations)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	float ForwardMovement;
	
	// The right movement value (used to drive animations)
	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	float RightMovement;
	
	// The distance at which old weapons spawn during a weapon swap
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float WeaponSpawnDistance = 100.0f;
	
	// The maximum distance in unreal units at which the player can interact with an object
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float InteractDistance = 400.0f;

	// Integers

	// amount of traces to draw for vault calculations, to get distance in unreal units, multiply by 5
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	int VaultTraceAmount = 25.0f;

	// Variables for movement
	
	// The maximum speed of the character when in the sprint state
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 700.0f;
	
	// The maximum speed of the character when in the walk state
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 400.0f;
	
	// Determines the speed of the character when crouched
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchMovementSpeed = 250.0f;
	
	// Determines the speed of the character when sliding
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SlideSpeed = 1000.0f;
	
	// Vault transforms
	FTransform LocalTargetTransform;

	// Timer managers
	FTimerHandle SlideStop;
    
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
