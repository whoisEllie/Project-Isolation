// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->SetupAttachment(RootComponent);
    
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("meshComp"));
    MeshComp->CastShadow = false;
    MeshComp->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);
    
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
    CameraComp->SetupAttachment(MeshComp, "cameraSocket");
    
    isCrouching = false;
    
    crouchSpeed = 10.0f; // the speed at which the player crouches, can be overridden in BP_Character
    defaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
    finalCapsuleHalfHeight = 44.0f; // the desired final crouch height, can be overridden in BP_Character

    walkSpeed = 400.0f;
    sprintSpeed = 550.0f;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
            GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
        }
        isCrouching = true;
    }
    else
    {
        isCrouching = false;
    }
}

void ASCharacter::StartSprint()
{
    if (isCrouching)
    {
        isCrouching = false;
    }
    
    isSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
}

void ASCharacter::StopSprint()
{
    if (!isCrouching)
    {
        isSprinting = false;
        GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
    }
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Crouching
	// Sets the new Target Half Height based on whether the player is crouching or uncrouching
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
	
	// Sprinting (start/stop)
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::StopSprint);

}

