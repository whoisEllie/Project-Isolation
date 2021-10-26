// Fill out your copyright notice in the Description page of Project Settings.
//Testing again, just to make sure this bad boi is working >:D


#include "SCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SWeaponBase.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    // Spawning the spring arm component
    springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
    springArmComp->bUsePawnControlRotation = true;
    springArmComp->SetupAttachment(RootComponent);
    
    // Spawning the FPS hands mesh component and attaching it to the spring arm component
    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("meshComp"));
    meshComp->CastShadow = false;
    meshComp->AttachToComponent(springArmComp, FAttachmentTransformRules::KeepRelativeTransform);
    
    // Spawning the camera atop the FPS hands mesh
    cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
    cameraComp->SetupAttachment(meshComp, "cameraSocket");
    
    crouchSpeed = 10.0f; // the speed at which the player crouches, can be overridden in BP_Character
    defaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
    finalCapsuleHalfHeight = 44.0f; // the desired final crouch height, can be overridden in BP_Character

    crouchMovementSpeed = 250.0f; // the player's movement speed while crouched
    walkSpeed = 400.0f; // the player's regular movement speed
    sprintSpeed = 550.0f; // the player's movement speed while sprinting
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
    UpdateWeapon(primaryWeapon);
}

// Built in UE function for moving forward/back
void ASCharacter::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}

// Built in UE function for moving left/right
void ASCharacter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() * value);
}

// Built in UE function for looking up/down
void ASCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

// Built in UE function for looking left/right
void ASCharacter::LookRight(float value)
{
	AddControllerYawInput(value);
}

// Custom crouch function
void ASCharacter::ExecCrouch()
{
    // Code for when the player is not already crouching
    if (!isCrouching)
    {
        // If the player is sprinting, turn the sprinting variable off
        if(isSprinting)
        {
            isSprinting = false;
        }
        
        // Update the speed and set the player to crouch
        isCrouching = true;
        UpdateMovementSpeed();
    }

    // Code for when the player is crouching
    else
    {
        // Uncrouch the player and update the movement speed
        isCrouching = false;
        UpdateMovementSpeed();
    }
}

// Starting to sprint (IE_Pressed)
void ASCharacter::StartSprint()
{
    // Disables crouch if the player was crouching
    if (isCrouching)
    {
        isCrouching = false;
    }
    
    // Updates the sprint speed
    isSprinting = true;
    UpdateMovementSpeed();
}

// Stopping to sprint (IE_Released)
void ASCharacter::StopSprint()
{
    // Updates sprint speed
    isSprinting = false;
    UpdateMovementSpeed();
}

// Function that determines the player's maximum speed, based on whether they're crouching, sprinting or neither
void ASCharacter::UpdateMovementSpeed()
{
    if (isCrouching)
    {
        GetCharacterMovement()->MaxWalkSpeed = crouchMovementSpeed;
    }
    if (isSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
    }
    if (!isSprinting && !isCrouching)
    {
        GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
    }
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void ASCharacter::UpdateWeapon(TSubclassOf<ASWeaponBase> newWeapon)
{
    // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters spawnParams;
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (currentWeapon)
    {
        // Destroys the current weapon, if it exists
        currentWeapon->K2_DestroyActor();
    }
    // Spawns the new weapon and sets the player as it's owner
    currentWeapon = GetWorld()->SpawnActor<ASWeaponBase>(newWeapon, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams);
    if (currentWeapon)
    {
        currentWeapon->SetOwner(this);
        currentWeapon->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, currentWeapon->weaponAttachmentSocketName);
    }   
}

// Lazy solution - read below
void ASCharacter::SwapToPrimary()
{
    UpdateWeapon(primaryWeapon);
}

// Lazy solution - read below
void ASCharacter::SwapToSecondary()
{
    UpdateWeapon(secondaryWeapon);
}

// Passing player inputs to SWeaponBase
void ASCharacter::StartFire()
{
    currentWeapon->StartFire();
}

// Passing player inputs to SWeaponBase
void ASCharacter::StopFire()
{
    currentWeapon->StopFire();
}

// Passing player inputs to SWeaponBase
void ASCharacter::Reload()
{
    currentWeapon->Reload();
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Crouching
	// Sets the new Target Half Height based on whether the player is crouching or standing
	float targetHalfHeight = isCrouching? finalCapsuleHalfHeight : defaultCapsuleHalfHeight;
	// Interpolates between the current height and the target height
	float newHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), targetHalfHeight, DeltaTime, crouchSpeed);
	// Sets the half height of the capsule component to the new interpolated half height
	GetCapsuleComponent()->SetCapsuleHalfHeight(newHalfHeight);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Move forward/back + left/right inputs
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	
	// Look up/down + left/right
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::LookRight);
	
	// Crouching
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::ExecCrouch);
	
	// Sprinting
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::StopSprint);
	
	// Jumping
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

    // Weapon swap
    /* There has to be a better way of doing this, passing a parameter through an input so I can just do
    &ASCharacter::UpdateWeapon(ASWeaponBase* NewWeapon) but after tying a bunch of solutions I couldn't find anything.
    If you know how to do it pleasepleasepleaseplease fix it in a branch and i'll merge them, thanks!
    For now, i've used the ugly solution of having additional functions which just pass through to UpdateWeapon
    with a primaryWeapon and secondaryWeapon argument*/
    PlayerInputComponent->BindAction("PrimaryWeapon", IE_Pressed, this, &ASCharacter::SwapToPrimary);
    PlayerInputComponent->BindAction("SecondaryWeapon", IE_Pressed, this, &ASCharacter::SwapToSecondary);

    // Firing
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

    // Reloading
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::Reload);
}
