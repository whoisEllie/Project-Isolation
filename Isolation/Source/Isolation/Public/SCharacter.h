// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;
class ASWeaponBase;


UCLASS()
class ISOLATION_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

    //Hands mesh, assignable through blueprints
    UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComp;
	//Camera Comp - component for the FPS camera
	UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;
	//Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation'
	UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
    
    
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
	// Overriding the built in crouch function
	void ExecCrouch();
	// Starting to sprint
	void StartSprint();
	// Stopping to sprint
	void StopSprint();
	// Global system to update movement speed
	void UpdateMovementSpeed();
	
	// Booleans
	
	// true if the player is crouching, false if not
	bool isCrouching;
	// true if the player is sprinting, false if not
	bool isSprinting;

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
	
	// Variables for sprint
	// The maximum speed of the character when in the sprint state
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float sprintSpeed;
	// The maximum speed of the character when in the walk state
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float walkSpeed;
	// Determines the speed of the character when crouched
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float crouchMovementSpeed;
    
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
