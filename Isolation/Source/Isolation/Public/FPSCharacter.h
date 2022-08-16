// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSCharacterController.h"
#include "Widgets/HUDWidget.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Widgets/PauseWidget.h"
#include "Widgets/SettingsWidget.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "WeaponBase.h"
#include "FPSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ASWeaponBase;
class UAnimMontage;
class UCurveFloat;
class UBlendSpace;

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

/** Variables associated with each movement state */
USTRUCT()
struct FMovementVariables
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	bool bCanFire;

	UPROPERTY(EditDefaultsOnly)
	float MaxAcceleration;

	UPROPERTY(EditDefaultsOnly)
	float BreakingDecelerationWalking;

	UPROPERTY(EditDefaultsOnly)
	float GroundFriction;

	UPROPERTY(EditDefaultsOnly)
	float MaxWalkSpeed;
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
	FText GetCurrentWeaponLoadedAmmo() const
	{
		if (CurrentWeapon != nullptr)
		{
			return FText::AsNumber(CurrentWeapon->GeneralWeaponData.ClipSize);
		}
		return FText::AsNumber(0);
	} 

	/** Returns the amount of ammunition remaining for the current weapon */
	UFUNCTION(BlueprintCallable)
	FText GetCurrentWeaponRemainingAmmo() const
	{
		ASCharacterController* CharacterController = Cast<ASCharacterController>(GetController());

		if (CharacterController)
		{
			if (CurrentWeapon != nullptr)
			{
				return FText::AsNumber(CharacterController->AmmoMap[CurrentWeapon->GeneralWeaponData.AmmoType]);
			}
			return FText::AsNumber(0);
		}
		return FText::FromString("No Character Controller found");
	}

	/** Switching to a new weapon
	 * @param NewWeapon The new weapon which to spawn
	 * @param InventoryPosition The position in the player's inventory in which to place the weapon
	 * @param bSpawnPickup Whether to spawn a pickup of CurrentWeapon (can be false if player has an empty weapon slot)
	 * @param bStatic Whether the spawned pickup should implement a physics simulation or not
	 * @param PickupTransform The position at which to spawn the new pickup, in the case that it is static (bStatic)
	 * @param DataStruct The FRuntimeWeaponData struct for the newly equipped weapon
	 * 
	 */
	void UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, int InventoryPosition, bool bSpawnPickup,
	                  bool bStatic, FTransform PickupTransform, FRuntimeWeaponData DataStruct);

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
	
	/** Returns the character's current movement state */
	EMovementState GetMovementState() const { return MovementState; }

	/** Returns the display text of the current interactable object that the player is looking at */
	UFUNCTION(BlueprintCallable)
	FText& GetInteractText() { return InteractText; }

	/** Returns an equipped weapon
	 *	@param WeaponID The ID of the weapon to get
	 *	@return The weapon with the given ID
	 */
	ASWeaponBase* GetWeaponByID(const int WeaponID) const { return EquippedWeapons[WeaponID]; }

	/** Returns the character's hands mesh */
	USkeletalMeshComponent* GetHandsMesh() const { return HandsMeshComp; }

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

	/** Draws the settings menu to the screen */
	UFUNCTION(BlueprintCallable)
	void CreateSettingsMenu();

	/** Returns the character's current input mapping context */
	UFUNCTION(BlueprintCallable)
	UInputMappingContext* GetBaseMappingContext() const { return BaseMappingContext; }

	/** Returns the number of weapon slots */
	int GetNumberOfWeaponSlots() const { return NumberOfWeaponSlots; }

	/** Returns the currently equipped weapon slot */
	int GetCurrentWeaponSlot() const { return CurrentWeaponSlot; }

	/** Returns the map of currently equipped weapons */
	TMap<int, ASWeaponBase*> GetEquippedWeapons() const { return EquippedWeapons; }
	
protected:

	/** The character's FPS camera component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	UCameraComponent* CameraComp;

	/** The character's hands mesh component */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USkeletalMeshComponent* HandsMeshComp;
	
	/** The spring arm component, which is required to enable 'use control rotation' */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	/** The component used to play audio for footsteps */
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UAudioComponent* FootstepAudioComp;
	
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

	/** Hand montage, played during vault */
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	UAnimMontage* VaultMontage;


private:

#pragma region FUNCTIONS

	/** Sets default values for this character's properties */
	AFPSCharacter();

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	virtual void PawnClientRestart() override;
	
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
	
	/** Function that actually executes the Vault
	 * @param TargetTransform The location to which to interpolate the player
	 */
	void Vault(FTransform TargetTransform);
	
	/** A global system that handles updates to the movement state and changes relevant values accordingly
	 *	@param NewMovementState The new movement state of the player
	 */
	void UpdateMovementValues(EMovementState NewMovementState);

	/** Swap to a new weapon
	 *	@param SlotId The ID of the slot which to swap to
	 */
	void SwapWeapon(int SlotId);

	/**	Template function for SwapWeapon (used with the enhanced input component) */
	template <int SlotID>
	void SwapWeapon() { SwapWeapon(SlotID); }
	
	/** Swaps between weapons using the scroll wheel */
	void ScrollWeapon(const FInputActionValue& Value);

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
	UFUNCTION()
	void TimelineProgress(float Value);

	/** Interaction with the world using SInteractInterface */
	void WorldInteract();

	/** Displaying the indicator for interaction */
	void InteractionIndicator();

	/** Updates widgets */
	void ManageOnScreenWidgets();

	/** Move the character left/right and forward/back
	 *	@param Value The value passed in by the Input Component
	 */
	void Move(const FInputActionValue& Value);

	/** Look left/right and up/down
	 *	@param Value The value passed in by the Input Component
	 */
	void Look(const FInputActionValue& Value);
		
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

#pragma endregion 

#pragma region USER_VARIABLES
		
	/** Prints debug variables if true */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bDrawDebug;
	
	/** Sets the height of the player's capsule component when crouched */	
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Crouch")
	float CrouchedCapsuleHalfHeight = 58.0f;
	
	/** The change in height of the spring arm that the camera + hands rest on when the player is crouched
	 *
	 *	In order to best line up with the crouched height, this should be equal to the
	 *	CrouchedCapsuleHalfHeight minus the capsule's default height. For example, if the default capsule half height
	 *	is 88.0f, and the crouched half height is 58.0f, then the crouched spring arm height delta should be -30
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Crouch")
	float CrouchedSpringArmHeightDelta = -30.0f;

	/** The default offset of the spring arm from a Z position of 0, set automatically on BeginPlay */
	float DefaultSpringArmOffset;
	
	//* The current offset of the spring arm */
	float CurrentSpringArmOffset = 0.0f;
	
	/** The rate at which the character crouches */
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Crouch") 
	float CrouchSpeed = 10.0f;
	
	/** The time in seconds between the beginning of a slide and when it is ended */
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Slide")
	float SlideTime = 1.0f;

	/** The height of the highest surface that the player can mantle up onto */
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Vault")
	float MaxMantleHeight = 200.0f;
	
	/** The amount of traces to draw for vault collision checks, to get distance in unreal units, multiply by 5 */
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Vault")
	int VaultTraceAmount = 25.0f;

	/** Curve that controls motion during vault */
	UPROPERTY(EditAnywhere, Category = "Movement | Vault")
	UCurveFloat* VaultTimelineCurve;
	
	/** A map holding data for each movement state */
	UPROPERTY(EditDefaultsOnly, Category = "Movement | Data")
	TMap<EMovementState, FMovementVariables> MovementDataMap;
	
	/** The game's default FOV */
	UPROPERTY(EditDefaultsOnly, Category = "Camera | FOV")
	float BaseFOV = 77.0f;
	
	/** The speed at which FOV changes occur */
	UPROPERTY(EditDefaultsOnly, Category = "Camera | FOV")
	float FOVChangeSpeed = 2.0f;
	
	/** The increase in FOV during fast actions, such as sprinting and sliding */
	UPROPERTY(EditDefaultsOnly, Category = "Camera | FOV")
	float SpeedFOVChange = 5.0f;

	/** The distance at which pickups for old weapons spawn during a weapon swap */
	UPROPERTY(EditDefaultsOnly, Category = "Camera | Interaction")
	float WeaponSpawnDistance = 100.0f;
	
	/** The maximum distance in unreal units at which the player can interact with an object */
	UPROPERTY(EditDefaultsOnly, Category = "Camera | Interaction")
	float InteractDistance = 400.0f;
	
	/** Name of the socket we attach our camera to */
	UPROPERTY(EditDefaultsOnly, Category = "Camera | Socket")
	FName CameraSocketName;
	
	/** THe Number of slots for Weapons that this player has */
	UPROPERTY(EditDefaultsOnly, Category = "Weapons | Inventory")
	int NumberOfWeaponSlots;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<USHUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<UPauseWidget> PauseWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<USettingsWidget> SettingsWidget;

	/** The material parameter collection that stores the scope opacity parameter to be changed */
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialParameterCollection* ScopeOpacityParameterCollection;

	/** The name of the parameter to modify in the material parameter collection */
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	FName OpacityParameterName;

	/** Array of physical materials for footsteps */
	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TArray<UPhysicalMaterial*> SurfaceMaterialArray;

#pragma endregion 

#pragma region INTERNAL_VARIABLES
	
	/** A Map storing the player's current weapons and the slot that they correspond to */
	UPROPERTY()
	TMap<int, ASWeaponBase*> EquippedWeapons;

	/** The player's currently equipped weapon */
	UPROPERTY()
	ASWeaponBase* CurrentWeapon;
	
	/** Enumerator holding the 5 possible movement states defined by EMovementState */
	UPROPERTY()
    EMovementState MovementState;
	
	/** The current message to be displayed above the screen (if any) */
	UPROPERTY()
	FText InteractText;
	
	/** A reference to the player's main HUD widget */
	UPROPERTY()
	USHUDWidget* PlayerHudWidget;

	/** A reference to the player's pause widget */
	UPROPERTY()
	UPauseWidget* PlayerPauseWidget;

	/** A reference to the player's settings widget */
	UPROPERTY()
	USettingsWidget* PlayerSettingsWidget;

	/** The current widget visible on screen */
	UPROPERTY()
	UUserWidget* CurrentWidget;
	
	/** The timeline for vaulting (generated from the curve) */
	UPROPERTY()
	FTimeline VaultTimeline;
	
	/** Hit results for various line traces */
	FHitResult FootstepHit;

	FHitResult StandUpHit;
	
	FHitResult VaultHit;

	FHitResult AngleHit;

	FHitResult InteractionHit;
	
	/** Whether the player is holding down the aim down sights button */
	bool bWantsToAim;
	
	/** Whether we should display a crosshair or not */
	bool bShowCrosshair;
	
	/** Whether the player is actually aiming down sights */
	bool bIsAiming;
	
	/** Whether we are currently vaulting or not */
	bool bIsVaulting;
	
	/** Whether the player is holding the crouch button */
	bool bHoldingCrouch;
	
	/** Whether the character has performed a slide yet? */
	bool bPerformedSlide;
	
	/** Whether the player is holding the sprint key */
	bool bHoldingSprint;
	
	/** Whether the player wants to slide (is holding the crouch/slide key, but not on the ground) */
	bool bWantsToSlide;
	
	/** Whether the character is sprinting */
	bool bIsSprinting;
	
	/** Whether the character is crouching */
	bool bIsCrouching;
	
	/** Whether the object we are looking at is one we are able to interact with (used for UI) */
	bool bCanInteract;
	
	/** Whether the interaction object the character is looking at is a weapon pickup (used for UI) */
	bool bInteractionIsWeapon;
	
	/** The integer that keeps track of which weapon slot ID is currently active */
	int CurrentWeaponSlot;

	/** Keeps track of the opacity of scopes */
	float ScopeBlend;
	
	/** The start location of a vaulting or mantle */
	FTransform VaultStartLocation;

	/** The end location of a vault or mantle */
	FTransform VaultEndLocation;
	
	/** Set automatically, the base height of the capsule */
	float DefaultCapsuleHalfHeight;

	/** The current angle of the floor beneath the player */
	float FloorAngle;
	
	/** The forward movement value (used to drive animations) */
	float ForwardMovement;
	
	/** The right movement value (used to drive animations) */
	float RightMovement;

	/** The look up value (used to drive procedural weapon sway) */
	float MouseY;

	/** The right look value (used to drive procedural weapon sway) */
	float MouseX;
	
	/** The target location of a vault or mantle */
	FTransform VaultTargetLocation;

	/** Timer manager for sliding */
	FTimerHandle SlideStop;

#pragma endregion 

#pragma region INPUT

	/** Input actions */

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* MovementAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* FiringAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* PrimaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* SecondaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* ReloadAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* AimAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InteractAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* ScrollAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* PauseAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* RemapAction;

	/** Input Mappings */

	UPROPERTY(EditDefaultsOnly, Category = "Input | Mappings")
	UInputMappingContext* BaseMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Mappings")
	int32 BaseMappingPriority = 0;

	/** Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma endregion
};
