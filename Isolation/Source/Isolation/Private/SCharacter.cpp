// Fill out your copyright notice in the Description page of Project Settings.


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
    
    springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
    springArmComp->bUsePawnControlRotation = true;
    springArmComp->SetupAttachment(RootComponent);
    
    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("meshComp"));
    meshComp->CastShadow = false;
    meshComp->AttachToComponent(springArmComp, FAttachmentTransformRules::KeepRelativeTransform);
    
    cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
    cameraComp->SetupAttachment(meshComp, "cameraSocket");
    
    crouchSpeed = 10.0f; // the speed at which the player crouches, can be overridden in BP_Character
    defaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
    finalCapsuleHalfHeight = 44.0f; // the desired final crouch height, can be overridden in BP_Character

    crouchMovementSpeed = 250.0f;
    walkSpeed = 400.0f;
    sprintSpeed = 550.0f;
    bCanWeaponSwap = true;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
    UpdateWeapon(primaryWeapon);
}

void ASCharacter::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}

void ASCharacter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() * value);
}

void ASCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

void ASCharacter::LookRight(float value)
{
	AddControllerYawInput(value);
}

void ASCharacter::ExecCrouch()
{
    if (!isCrouching)
    {
        if(isSprinting)
        {
            isSprinting = false;
        }
        
        isCrouching = true;
        UpdateMovementSpeed();
    }
    else
    {
        isCrouching = false;
        UpdateMovementSpeed();
    }
}

void ASCharacter::StartSprint()
{
    if (isCrouching)
    {
        isCrouching = false;
    }
    
    isSprinting = true;
    UpdateMovementSpeed();
}

void ASCharacter::StopSprint()
{
    if (!isCrouching)
    {
        isSprinting = false;
        UpdateMovementSpeed();
    }
}

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

void ASCharacter::UpdateWeapon(TSubclassOf<ASWeaponBase> newWeapon)
{
    if (bCanWeaponSwap)
    {
        FActorSpawnParameters spawnParams;
        spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        if (currentWeapon)
        {
            currentWeapon->K2_DestroyActor();
        }
        currentWeapon = GetWorld()->SpawnActor<ASWeaponBase>(newWeapon, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams);
        if (currentWeapon)
        {
            currentWeapon->SetOwner(this);
            currentWeapon->AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, currentWeapon->weaponAttachmentSocketName);
        }   
    }
}

void ASCharacter::SwapToPrimary()
{
    UpdateWeapon(primaryWeapon);
}

void ASCharacter::SwapToSecondary()
{
    UpdateWeapon(secondaryWeapon);
}

void ASCharacter::StartFire()
{
    currentWeapon->StartFire();
}

void ASCharacter::StopFire()
{
    currentWeapon->StopFire();
}

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
    PlayerInputComponent->BindAction("PrimaryWeapon", IE_Pressed, this, &ASCharacter::SwapToPrimary);
    PlayerInputComponent->BindAction("SecondaryWeapon", IE_Pressed, this, &ASCharacter::SwapToSecondary);

    // Firing
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

    // Reloading
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::Reload);
}

