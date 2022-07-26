// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

/** Movement state enumerator holding all possible movement states */
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	State_Walk      UMETA(DisplayName = "Walking"),
	State_Sprint    UMETA(DisplayName = "Sprinting"),
	State_Crouch    UMETA(DisplayName = "Crouching"),
	State_Slide		UMETA(DisplayName = "Sliding"),
	State_Vault	    UMETA(DisplayName = "Vaulting")
};

/** Struct keeping track of important weapon variables for the primary/secondary weapons */
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

	/**  Returns the amount of ammunition currently loaded into the weapon  */
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponLoadedAmmo() const;

	/** Returns the amount of ammunition remaining for the current weapon */
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponRemainingAmmo() const;

	/** Switching to a new weapon
	 * @param NewWeapon The new weapon which to spawn
	 * @param bSpawnPickup Whether to spawn a pickup of CurrentWeapon (can be false if player has an empty weapon slot)
	 * @param CurrentDataStruct The current struct of weapon data, used to assign data to the pickup
	 * @param bStatic Whether the spawned pickup should implement a physics simulation or not
	 * @param PickupTransform The position at which to spawn the new pickup, in the case that it is static (bStatic)
	 */
	void UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, bool bSpawnPickup, FWeaponDataStruct* CurrentDataStruct,
	                  bool bStatic, FTransform PickupTransform);

	/** Returns the character's forward movement (from 0 to 1) */
	UFUNCTION(BlueprintCallable)
	float GetForwardMovement() const { return ForwardMovement; }

	/** Returns the character's sideways movement (from 0 to 1) */
	UFUNCTION(BlueprintCallable)
	float GetRightMovement() const { return RightMovement; }

	/** Returns the character's vertical mouse position (from 0 to 1) */
	UFUNCTION(BlueprintCallable)
	float GetMouseY() const { return MouseY; }

	/** Returns the character's horizontal mouse position (from 0 to 1) */
	UFUNCTION(BlueprintCallable)
	float GetMouseX() const { return MouseX; }

	/** Returns the current base FOV (without temporary modifications such as ADS or Sprint) */
	UFUNCTION(BlueprintCallable)
	float GetBaseFOV() const { return BaseFOV; }

	/** Update the base FOV
	 * @param NewFOV The FOV to use as a new BaseFOV
	 */
	UFUNCTION(BlueprintCallable)
	void SetBaseFOV(const float NewFOV) { BaseFOV = NewFOV; }

	/** Returns the current weapon equipped by the player */
	UFUNCTION(BlueprintCallable)
	ASWeaponBase* GetCurrentWeapon() const {return CurrentWeapon; }

	/** Returns the current visibility of the crosshair */
	UFUNCTION(BlueprintCallable)
	bool IsCrosshairVisible() const { return bShowCrosshair; }

	/** Update the visibility of the crosshair in HUD UI component
	 * @param bVisible The new visibility of the crosshair
	 */
	UFUNCTION(BlueprintCallable)
	void SetCrosshairVisibility(const bool bVisible) { bShowCrosshair = bVisible; }

	/** Returns whether the player is currently aiming or not */
	UFUNCTION(BlueprintCallable)
	bool IsPlayerAiming() const { return bIsAiming; }

	/** Returns whether the player is sprinting or not */
	UFUNCTION(BlueprintCallable)
	bool IsPlayerSprinting() const { return bIsSprinting; }

	/** Returns whether the player is crouching or not */
	bool IsPlayerCrouching() const { return bIsCrouching; }

	/** Returns the result of the interaction trace, which is true if the object that we are looking at is able to be
	 *  interacted with */
	UFUNCTION(BlueprintCallable)
	bool CanInteract() const { return bCanInteract; }

	/** Returns true if the interaction trace is hitting a weapon pickup */
	bool InteractionIsWeapon() const { return bInteractionIsWeapon; }

	/** Returns true if the current weapon is the primary weapon */
	bool IsPrimaryWeaponEquipped() const { return bIsPrimary; }

	/** Updates the weapon the player is holding, where true = primary weapon and false = secondary weapon
	 *  @param bIsPrimaryWeaponEquipped Whether the equipped weapon should be the primary weapon
	 */
	void SetPrimaryWeaponEquipped(bool const bIsPrimaryWeaponEquipped) { bIsPrimary = bIsPrimaryWeaponEquipped; } 

	/** Returns the character's current movement state */
	EMovementState GetMovementState() const { return MovementState; }

	/** Returns the display text of the current interactable object that the player is looking at */
	UFUNCTION(BlueprintCallable)
	FText& GetInteractText() { return InteractText; }

	/** Returns the current primary weapon */
	TSubclassOf<ASWeaponBase> GetPrimaryWeapon() const { return PrimaryWeapon; }

	/** Updates the reference to the primary weapon
	 *  @param NewWeapon The new primary weapon
	 */
	void SetPrimaryWeapon(TSubclassOf<ASWeaponBase> const NewWeapon) { PrimaryWeapon = NewWeapon; }

	/** Returns the current secondary weapon */
	TSubclassOf<ASWeaponBase> GetSecondaryWeapon() const { return SecondaryWeapon; }

	/** Updates the reference to the secondary weapon
	 *  @param NewWeapon The new secondary weapon
	 */
	void SetSecondaryWeapon(TSubclassOf<ASWeaponBase> const NewWeapon) { SecondaryWeapon = NewWeapon; }

	/** Returns the character's hands mesh */
	USkeletalMeshComponent* GetHandsMesh() const { return HandsMeshComp; }

	/** Returns a reference to the primary weapon's cached data map */
	FWeaponDataStruct* GetPrimaryWeaponCacheMap() { return &PrimaryWeaponCacheMap; }

	/** Updates the primary weapon's cached data map
	 *	@param NewWeaponDataStruct The new FWeaponDataStruct
	 */
	void SetPrimaryWeaponCacheMap(const FWeaponDataStruct* const NewWeaponDataStruct)
	{
		PrimaryWeaponCacheMap = *NewWeaponDataStruct;
	}

	/** Returns a reference to the secondary weapon's cached data map */
	FWeaponDataStruct* GetSecondaryWeaponCacheMap() { return &SecondaryWeaponCacheMap; }

	/** Updates the secondary weapon's cached data map
	 *	@param NewWeaponDataStruct The new FWeaponDataStruct
	 */
	void SetSecondaryWeaponCacheMap(const FWeaponDataStruct* const NewWeaponDataStruct)
	{
		SecondaryWeaponCacheMap = *NewWeaponDataStruct;
	}

	/** Returns a reference to the player's heads up display */
	USHUDWidget* GetPlayerHud() const { return PlayerHudWidget; }

	/** Returns a reference to the player's camera component */
	UCameraComponent* GetCameraComponent() const { return CameraComp; }

	/** Returns the character's empty-handed walking blend space */
	UFUNCTION(BlueprintCallable)
	UBlendSpace* GetWalkBlendSpace() const { return BS_Walk; }

	/** Returns the character's empty-handed walking blend space for use in the aiming state */
	UFUNCTION(BlueprintCallable)
	UBlendSpace* GetWalkAdsBlendSpace() const { return BS_Ads_Walk; }

	/** Returns the character's empty-handed idle animation */
	UFUNCTION(BlueprintCallable)
	UAnimSequence* GetIdleAnim() const { return Anim_Idle; }

	/** Returns the character's empty handed idle animation for use in the aiming state */
	UFUNCTION(BlueprintCallable)
	UAnimSequence* GetAdsIdleAnim() const { return Anim_Ads_Idle; }

	/** Returns the character's empty handed sprinting animation */
	UFUNCTION(BlueprintCallable)
	UAnimSequence* GetSprintAnim() const { return Anim_Sprint; }	
	
protected:

	/** The character's FPS camera component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	UCameraComponent* CameraComp;

	/**  The character's hands mesh component */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* HandsMeshComp;
	
	/** Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation' */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
	
	/** Hand animation blend space for when the player has no weapon  */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UBlendSpace* BS_Walk;

	/** Hand animation blend space for then the player has no weapon and is aiming down sights */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UBlendSpace* BS_Ads_Walk;

	/** Hand animation for when the player has no weapon and is idle */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Idle;

	/** Hand animation for when the player has no weapon, is idle, and is aiming down sights */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Animations")
	UAnimSequence* Anim_Ads_Idle;

	/** Hand animation for when the player has no weapon and is sprinting */
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

	/** Alternative to the built in Crouch function
	 *  Handles crouch input and decides what action to perform based on the character's current state
	 */
	void StartCrouch();
	
	/** Transitions the character out of the crouched state
	 *	@param bToSprint Whether to transition into a sprint state
	 */
	void StopCrouch(bool bToSprint);

	/** Exits the character from the slide state if they are sliding and updates bHoldingCrouch */
	void ReleaseCrouch();
	
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
	
	/** A global system that handles updates to the movement state and changes relevant values accordingly
	 *	@param NewMovementState The new movement state of the player
	 */
	void UpdateMovementValues(EMovementState NewMovementState);

	/** Temp function for swap to primary */
	void SwapToPrimary();

	/** Temp function for swap to secondary */
	void SwapToSecondary();
	
	/** Swaps between the primary and secondary weapon using the scroll wheel */
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
	void CheckAngle(float DeltaTime);

	/** Trace above the player to make sure we have enough space to stand up */
	bool HasSpaceToStandUp();

	/** Ticks the timeline */
	void TimelineProgress(float Value);

	/** Interaction with the world using SInteractInterface */
	void WorldInteract();

	/** Displaying the indicator for interaction */
	void InteractionIndicator();

	/** Debug */
	
	/** Prints debug variables if true */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug;

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

	/** Sets the height of the player's capsule component when crouched */	
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float CrouchedCapsuleHalfHeight = 58.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	/** The change in height of the spring arm that the camera + hands rest on when the player is crouched
	 *	@warning { In order to best line up with the crouched height, this should be equal to the
	 *	CrouchedCapsuleHalfHeight minus the capsule's default height. For example, if the default capsule half height
	 *	is 88.0f, and the crouched half height is 58.0f, then the crouched spring arm height delta should be -30 }
	 */
	float CrouchedSpringArmHeightDelta = -30.0f;
	
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
	USHUDWidget* PlayerHudWidget;

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
	UPROPERTY()
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
