// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ASWeaponBase;

UENUM(BlueprintType)
enum MovementState
{
	VE_Walk         UMETA(DisplayName = "Walking"),
	VE_Sprint       UMETA(DisplayName = "Sprinting"),
	VE_Crouch       UMETA(DisplayName = "Crouching"),
	VE_Slide		UMETA(DisplayName = "Sliding"),
	VE_Vault	    UMETA(DisplayName = "Vaulting")
};


UCLASS()
class ISOLATION_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

    //Hands mesh, assignable through blueprints
    UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* meshComp;
	//Camera Comp - component for the FPS camera
	UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
	UCameraComponent* cameraComp;
	//Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation'
	UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* springArmComp;

	// Weapon classes
    
	// A reference to the player's current primary weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASWeaponBase> primaryWeapon;
	// A reference to the player's current secondary weapon
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASWeaponBase> secondaryWeapon;
	// The player's currently equipped weapon
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	ASWeaponBase* currentWeapon;


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

	// Enumerators

	// Enumerator holding the 4 possible movement states defined above
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement State")
    TEnumAsByte<MovementState> movementState;

	// Other
	// Name of the socket we attach our camera to
	UPROPERTY(EditDefaultsOnly, Category = "Other")
	FName cameraSocketName;
    
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
	void EndCrouch(bool toSprint);

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
	void Vault(float height, FTransform targetTransform);
	// Global system to update movement speed
	void UpdateMovementSpeed();

	// Switches to new weapon
	void UpdateWeapon(TSubclassOf<ASWeaponBase> newWeapon);

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

	FHitResult hit;

	// Floats
	
	// Variables for crouch system
	// Sets the height of the player when crouched
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float finalCapsuleHalfHeight;
	// Set in the default values, the base height of the capsule
	float defaultCapsuleHalfHeight;
	// Determines the rate at which the character crouches
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float crouchSpeed;
	// Slide time
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float slideTime;
	// Default FOV
	float defaultFOV;
	// target FOV
	float speedFOV;
	// change speed for the fov
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float fovChangeSpeed;
	// amount for FOV to increase
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float fovChangeAmount;


	// Variables for movement
	// The maximum speed of the character when in the sprint state
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float sprintSpeed;
	// The maximum speed of the character when in the walk state
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float walkSpeed;
	// Determines the speed of the character when crouched
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float crouchMovementSpeed;
	// Determines the speed of the character when sliding
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float slideSpeed;
	//
	FTransform localTargetTransform;
	


	// Timer managers
	FTimerHandle slideStop;
    
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
