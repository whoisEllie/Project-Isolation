// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <unicode/platform.h>

#include "CoreMinimal.h"
#include "HUDWidget.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "FPSCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ASWeaponBase;
class UAnimMontage;
class UCurveFloat;
class UBlendSpace;

/** Enumerator holding the 4 types of ammunition that weapons can use (used as part of the FsingleWeaponParams struct)
 * and to keep track of the total ammo the player has (ammoMap) */
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

	/** The maximum size of the player's magazine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int ClipCapacity; 

	/** The amount of ammunition currently in the magazine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int ClipSize;

	/** Enumerator holding the 4 possible ammo types defined above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Type")
	EAmmoType AmmoType;

	/** The current health of the weapon (degradation values are in the weapon class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float WeaponHealth;

	/** The attachments used in the current weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	TArray<FName> WeaponAttachments;
};

UCLASS()
class ISOLATION_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	/** Plays footstep sounds, called from animations with anim notify. */
	UFUNCTION(BlueprintCallable)
	void FootstepSounds();

	/** Functions for UI
	 *  Get the amount of ammunition currently loaded into the weapon  */
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponLoadedAmmo() const;

	/** Get the amount of ammunition remaining for the current weapon */
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponRemainingAmmo() const;

	/** Switching to a new weapon */
	void UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, bool bSpawnPickup, FWeaponDataStruct* OldDataStruct, bool bStatic, FTransform PickupTransform);

	UFUNCTION(BlueprintCallable)
	float GetForwardMovement() const { return ForwardMovement; }

	UFUNCTION(BlueprintCallable)
	float GetRightMovement() const { return RightMovement; }

	UFUNCTION(BlueprintCallable)
	float GetMouseY() const { return MouseY; }

	UFUNCTION(BlueprintCallable)
	float GetMouseX() const { return MouseX; }

	UFUNCTION(BlueprintCallable)
	float GetBaseFOV() const { return BaseFOV; }

	UFUNCTION(BlueprintCallable)
	void SetBaseFOV(const float NewFOV) { BaseFOV = NewFOV; }
	
	UFUNCTION(BlueprintCallable)
	ASWeaponBase* GetCurrentWeapon() const {return CurrentWeapon; }

	UFUNCTION(BlueprintCallable)
	bool GetCrosshairVisibility() const { return bShowCrosshair; }

	UFUNCTION(BlueprintCallable)
	void SetCrosshairVisibility(const bool Visible) { bShowCrosshair = Visible; }

	UFUNCTION(BlueprintCallable)
	bool GetAimingStatus() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable)
	bool GetSprintingStatus() const { return bIsSprinting; }

	bool GetCrouchingStatus() const { return bIsCrouching; }

	UFUNCTION(BlueprintCallable)
	bool CanInteract() const { return bCanInteract; }

	bool InteractionIsWeapon() const { return bInteractionIsWeapon; }

	bool IsPrimaryWeaponEquipped() const { return bIsPrimary; }

	void SetPrimaryWeaponEquipped(bool const bIsPrimaryWeaponEquipped) { bIsPrimary = bIsPrimaryWeaponEquipped; } 

	EMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintCallable)
	FText& GetInteractText() { return InteractText; }

	TSubclassOf<ASWeaponBase> GetPrimaryWeapon() const { return PrimaryWeapon; }

	void SetPrimaryWeapon(TSubclassOf<ASWeaponBase> const NewWeapon) { PrimaryWeapon = NewWeapon; }

	void SetSecondaryWeapon(TSubclassOf<ASWeaponBase> const NewWeapon) { SecondaryWeapon = NewWeapon; }

	TSubclassOf<ASWeaponBase> GetSecondaryWeapon() const { return SecondaryWeapon; }

	USkeletalMeshComponent* GetHandsMesh() const { return HandsMeshComp; }

	FWeaponDataStruct* GetPrimaryWeaponCacheMap() { return &PrimaryWeaponCacheMap; }

	void SetPrimaryWeaponCacheMap(const FWeaponDataStruct* const NewWeaponDataStruct) { PrimaryWeaponCacheMap = *NewWeaponDataStruct; }

	FWeaponDataStruct* GetSecondaryWeaponCacheMap() { return &SecondaryWeaponCacheMap; }

	void SetSecondaryWeaponCacheMap(const FWeaponDataStruct* const NewWeaponDataStruct) { SecondaryWeaponCacheMap = *NewWeaponDataStruct; }

	USHUDWidget* GetUserWidget() const { return UserWidget; }

	UCameraComponent* GetCameraComponent() const { return CameraComp; }

	UFUNCTION(BlueprintCallable)
	UBlendSpace* GetWalkBlendSpace() const { return BS_Walk; }

	UFUNCTION(BlueprintCallable)
	UBlendSpace* GetWalkAdsBlendSpace() const { return BS_Ads_Walk; }

	UFUNCTION(BlueprintCallable)
	UAnimSequence* GetIdleAnim() const { return Anim_Idle; }

	UFUNCTION(BlueprintCallable)
	UAnimSequence* GetAdsIdleAnim() const { return Anim_Ads_Idle; }

	UFUNCTION(BlueprintCallable)
	UAnimSequence* GetSprintAnim() const { return Anim_Sprint; }	
	
protected:

	/**Camera Comp - component for the FPS camera */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	UCameraComponent* CameraComp;

	/**Hands mesh, assignable through blueprints */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* HandsMeshComp;
	
	/**Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation' */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
	
	/** Hand animations for when the player has no weapon  */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UBlendSpace* BS_Walk;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UBlendSpace* BS_Ads_Walk;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Idle;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Ads_Idle;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Sprint;


private:

	/** Sets default values for this character's properties */
	AFPSCharacter();

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Moving forward/backwards (takes axis as input from CharacterMovementComponent.h) */
	void MoveForward(float Value);

	/** Moving left/right (takes axis as input from CharacterMovementComponent.h) */
	void MoveRight(float Value);

	/** Looking up/down (takes axis as input from CharacterMovementComponent.h) */
	void LookUp(float Value);

	/** Looking left/right (takes axis as input from CharacterMovementComponent.h) */
	void LookRight(float Value);

	/** Alternative to the built in Crouch function */
	void StartCrouch();

	/** Alternative to the built in UnCrouch function */
	void StopCrouch();

	/** We do this so that we can perform checks based on the height above the player (can they even stand up?) and to handle sliding */
	void EndCrouch(bool bToSprint);

	/** Starting to sprint */
	void StartSprint();

	/** Stopping to sprint */
	void StopSprint();

	/** Starting to slide */
	void StartSlide();

	/** Stopping to slide */
	void StopSlide();

	/** Function that runs on tick and checks if we should execute the Vault() functions */
	void CheckVault();
	
	/** Function that actually executes the Vault */
	void Vault(FTransform TargetTransform);
	
	/** Global system to update movement speed */
	void UpdateMovementValues(EMovementState NewMovementState);

	void SwapToPrimary();

	void SwapToSecondary();
	
	/** Changing the weapon with the scroll wheel */
	void ScrollWeapon();

	/** Fires the weapon */
	void StartFire();

	/** Stops firing the weapon */
	void StopFire();

	/** Reloads the weapon */
	void Reload();

	/** Starts ADS */
	void StartAds();

	/** Ends ADS */
	void StopAds();

	/** Checks the angle of the floor to determine slide behaviour */
	void CheckAngle();

	/** Trace above the player to make sure we have enough space to stand up */
	bool HasSpaceToStandUp();

	/** Ticks the timeline */
	UFUNCTION()
	void TimelineProgress(float value);

	/** Interaction with the world using SInteractInterface */
	void WorldInteract();

	/** Displaying the indicator for interaction */
	void InteractionIndicator();

	/** Variables for movement */
	
	/** The maximum speed of the character when in the sprint state */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 700.0f;
	
	/** The maximum speed of the character when in the walk state */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 400.0f;
	
	/** Determines the speed of the character when crouched */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchMovementSpeed = 250.0f;
	
	/** Determines the speed of the character when sliding */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SlideSpeed = 1000.0f;

	/** Sets the height of the player when crouched */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float CrouchedCapsuleHalfHeight = 68.0f; // the desired final crouch height, can be overridden in BP_Character

	/** Sets the height of the spring arm that the camera + hands rest on when the player is crouched */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float CrouchedSpringArmHeight = -30.0f;
	
	float CurrentSpringArmOffset = 0.0f;
	
	/** Determines the rate at which the character crouches */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float CrouchSpeed = 10.0f;
	
	/** Slide time */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float SlideTime = 1.0f;

	/** change speed for the fov */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FOVChangeSpeed = 2.0f;
	
	/** amount for FOV to increase */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float FOVChangeAmount = 5.0f;

	/** The height of the highest surface that the player can mantle up onto */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float MaxVaultHeight = 200.0f;
	
	/** The distance at which old weapons spawn during a weapon swap */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float WeaponSpawnDistance = 100.0f;
	
	/** The maximum distance in unreal units at which the player can interact with an object */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float InteractDistance = 400.0f;

	/** amount of traces to draw for vault calculations, to get distance in unreal units, multiply by 5 */
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	int VaultTraceAmount = 25.0f;
	
	/** Weapon classes */
	
	/** A reference to the player's current primary weapon */
	UPROPERTY()
	TSubclassOf<ASWeaponBase> PrimaryWeapon;
	
	/** A reference to the player's current secondary weapon */
	UPROPERTY()
	TSubclassOf<ASWeaponBase> SecondaryWeapon;

	/** The player's currently equipped weapon */
	UPROPERTY()
	ASWeaponBase* CurrentWeapon;

	/** Booleans */
	
	/** The boolean holding whether the player wants to aim or not */
	bool bWantsToAim;

	/** Getters + Setters
	 *  Whether we should render a crosshair or not */
	bool bShowCrosshair;
	
	/** Getters + Setters
	 *  The boolean holding whether we are aiming or not */
	bool bIsAiming;
	
	/** The boolean keeping track of whether we're vaulting or not */
	bool bIsVaulting;
	
	/** Keeps track whether the player is holding the Crouch button */
	bool bHoldingCrouch;
	
	/** Have we performed a slide yet? */
	bool bPerformedSlide;
	
	/** Is holding the sprint key */
	bool bHoldingSprint;
	
	/** wants to slide? (is holding the crouch/slide key, but not on the ground) */
	bool bWantsToSlide;
	
	/** prints debug variables if enabled */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug;

	/** Getters + Setters
	 *  keeps track of whether we're sprinting (for animations) */
	bool bIsSprinting;
	
	/** Getters + Setters
	 *  keeps track of whether we're crouching (for animations) */
	bool bIsCrouching;
	
	/** Getters + Setters
	 *  keeps track of whether the object we are looking at is one we are able to interact with (used for UI) */
	bool bCanInteract;
	
	/** Getters + Setters
	 *  keeps track of whether the interaction object is a weapon pickup (used for UI) */
	bool bInteractionIsWeapon;
	
	/** Whether the player is holding the primary weapon (or not, and are thus holding the secondary weapon) */
	bool bIsPrimary;

	/** Enumerators */
	
	/** Enumerator holding the 5 possible movement states defined above in EMovementState */
	UPROPERTY()
    EMovementState MovementState;

	/** UI */

	/** The current message to be displayed above the screen (if any) */
	UPROPERTY()
	FText InteractText;
	
	/** Other */
	
	/** Name of the socket we attach our camera to */
	UPROPERTY(EditDefaultsOnly, Category = "Other")
	FName CameraSocketName;

	/** Weapon variables */

	/** Weapon cache maps */
	UPROPERTY()
	FWeaponDataStruct PrimaryWeaponCacheMap;

	UPROPERTY()
	FWeaponDataStruct SecondaryWeaponCacheMap;
	
	UPROPERTY()
	USHUDWidget* UserWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UAudioComponent* FootstepAudioComp;

	/** keeps track of the opacity of scopes */
	float ScopeBlend;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialParameterCollection* ScopeOpacityParameterCollection;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	FName OpacityParameterName;

	/** collision parameters for spawning the line trace */
	FCollisionQueryParams QueryParams;

	/** Array of physical materials for footsteps */
	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TArray<UPhysicalMaterial*> SurfaceMaterialArray;

	/** The curve for vaulting */
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;
	
	/** The timeline for vaulting (generated from the curve) */
	FTimeline VaultTimeline;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USHUDWidget> HUDWidget;	
	
	/** Hit results for various line traces */
	FHitResult Hit;

	FHitResult VaultHit;

	FHitResult AngleHit;

	FHitResult InteractionHit;

	FTransform VaultStartLocation;

	FTransform VaultEndLocation;

	/** The montage for the hands vault animation  */
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* VaultMontage;
	
		
	/** Set in the default values, the base height of the capsule */
	float DefaultCapsuleHalfHeight;

	/** Set in the default values, the default relative location of the spring arm */
	FVector DefaultSpringArmHeight = FVector(0.0f, 0.0f, 90.0f);
	
	float BaseFOV;
	
	/** current angle of floor */
	float FloorAngle;
	
	/** vector of floor */
	FVector FloorVector;
	
	/** The forward movement value (used to drive animations) */
	float ForwardMovement;
	
	/** The right movement value (used to drive animations) */
	float RightMovement;

	/** The look up value (used to drive procedural weapon sway) */
	float MouseY;

	/** The right look value (used to drive procedural weapon sway) */
	float MouseX;
	
	/** Vault transforms */
	FTransform LocalTargetTransform;

	/** Timer managers */
	FTimerHandle SlideStop;
		
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
