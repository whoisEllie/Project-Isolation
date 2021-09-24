// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"

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

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

