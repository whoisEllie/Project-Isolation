// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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
}

void ASCharacter::TimelineProgress(float val)
{
    FVector newActorLocation = FMath::Lerp(VaultStartLocation.GetLocation(), VaultEndLocation.GetLocation(), val);
    SetActorLocation(newActorLocation);
    if (val == 1)
    {
        bIsVaulting = false;
    }
    
}   

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
    UpdateWeapon(primaryWeapon);
    defaultFOV = cameraComp->FieldOfView;
    speedFOV = defaultFOV + fovChangeAmount;

    if (curveFloat)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
        vaultTimeline.AddInterpFloat(curveFloat, TimelineProgress);
    }
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
void ASCharacter::StartCrouch()
{
    bHoldingCrouch = true;
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        if (movementState == VE_Crouch)
        {
            EndCrouch(false);
            //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Ending Crouch", true);
        }
        else if (movementState == VE_Sprint && !bPerformedSlide)
        {
            StartSlide();
        }
        else
        {
            movementState = VE_Crouch;
            UpdateMovementSpeed();
        }
    }
    else if (!bPerformedSlide)
    {
        bWantsToSlide = true;
    }
    
}

void ASCharacter::StopCrouch()
{
    bHoldingCrouch = false;    
    if (movementState == VE_Slide)
    {
        StopSlide();
        bPerformedSlide = false;
    }
}

void ASCharacter::EndCrouch(bool toSprint)
{
    if (movementState == VE_Crouch || movementState == VE_Slide)
    {
        //FVector centerVector = GetActorLocation();
        //centerVector.Z += 46;

        //FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(34.0f, defaultCapsuleHalfHeight);

        //DrawDebugCapsule(GetWorld(), centerVector, defaultCapsuleHalfHeight, 34.0f, FQuat::Identity, FColor::Red);

        //if (GetWorld()->SweepSingleByChannel(hit, centerVector, centerVector, FQuat::Identity, ECC_WorldStatic, CollisionCapsule))
        //{
        //    /* confetti or smth idk */
        //    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "SweepSingleByChannel returned true", true);
        //}
        if (toSprint)
        {
            movementState = VE_Sprint;
        }
        else
        {
            movementState = VE_Walk;
        }
        UpdateMovementSpeed();
    }
}

// Starting to sprint (IE_Pressed)
void ASCharacter::StartSprint()
{
    bHoldingSprint = true;
    // Updates the sprint speed
    movementState = VE_Sprint;
    UpdateMovementSpeed();
}

// Stopping to sprint (IE_Released)
void ASCharacter::StopSprint()
{
    if (movementState == VE_Slide && bHoldingCrouch)
    {
        movementState = VE_Crouch;
    }
    else
    {
        movementState = VE_Walk;
    }
    
    bHoldingSprint = false;
    UpdateMovementSpeed();
}

void ASCharacter::StartSlide()
{
    movementState = VE_Slide;
    bPerformedSlide = true;
    UpdateMovementSpeed();
    GetWorldTimerManager().SetTimer(slideStop, this, &ASCharacter::StopSlide, slideTime, false, slideTime);
}

void ASCharacter::StopSlide()
{
    bPerformedSlide = false;
    GetWorldTimerManager().ClearTimer(slideStop);
    if (movementState == VE_Slide)
    {
        if (bHoldingSprint)
        {
            EndCrouch(true);
        }
        else if (bHoldingCrouch)
        {
            movementState = VE_Crouch;
        }
        else
        {
            movementState = VE_Walk;
        }
        UpdateMovementSpeed();
    }
}

void ASCharacter::CheckVault()
{
    float forwardVelocity = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
    if (forwardVelocity > 0 && !bIsVaulting && GetCharacterMovement()->IsFalling())
    {
        FVector startLocation = GetCapsuleComponent()->GetComponentLocation();
        FVector endLocation = (GetCapsuleComponent()->GetComponentLocation() + (UKismetMathLibrary::GetForwardVector(GetCapsuleComponent()->GetComponentRotation()) * 75));
        //DrawDebugCapsule(GetWorld(), startLocation, 50, 30, FQuat::Identity, FColor::Red);

        FCollisionQueryParams TraceParams;
        TraceParams.bTraceComplex = true;
        TraceParams.AddIgnoredActor(this);

        if (GetWorld()->SweepSingleByChannel(hit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeCapsule(30, 50), TraceParams))
        {
            if (hit.bBlockingHit)
            {
                FVector forwardImpactPoint = hit.ImpactPoint;
                FVector forwardImpactNormal = hit.ImpactNormal;
                FVector capsuleLocation = forwardImpactPoint;
                capsuleLocation.Z = (GetCapsuleComponent()->GetComponentLocation().Z);
                capsuleLocation += (forwardImpactNormal * -15);
                startLocation = capsuleLocation;
                startLocation.Z += 100;
                endLocation = capsuleLocation;
                //DrawDebugSphere(GetWorld(), startLocation, 1, 4, FColor::Blue);
                if (GetWorld()->SweepSingleByChannel(hit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(1), TraceParams))
                {
                    if (GetCharacterMovement()->IsWalkable(hit))
                    {
                        FVector secondaryVaultStartLocation = hit.ImpactPoint;
                        secondaryVaultStartLocation.Z += 5;
                        DrawDebugSphere(GetWorld(), secondaryVaultStartLocation, 10, 8, FColor::Orange);
                        FRotator cacheRotator = GetCapsuleComponent()->GetComponentRotation();
                        FVector secondaryVaultEndLocation = secondaryVaultStartLocation;
                        secondaryVaultEndLocation.Z = 0;

                        float initialTraceHeight = 0;
                        float previousTraceHeight = 0;
                        float currentTraceHeight = 0;
                        bool initialSwitch = false;
                        bool vaultFailed = true;
                        
                        int i;
                        for (i = 0; i <= vaultTraceAmount; i++)
                        {
                            secondaryVaultStartLocation += (UKismetMathLibrary::GetForwardVector(cacheRotator) * 5);
                            secondaryVaultEndLocation += (UKismetMathLibrary::GetForwardVector(cacheRotator) * 5);
                            vaultFailed = true;
                            if(GetWorld()->LineTraceSingleByChannel(vaultHit, secondaryVaultStartLocation, secondaryVaultEndLocation, ECC_WorldStatic, TraceParams))
                            {
                                if (bDrawDebug)
                                {
                                    DrawDebugLine(GetWorld(), secondaryVaultStartLocation, vaultHit.ImpactPoint, FColor::Red, false, 10.0f, 0.0f, 2.0f);
                                }

                                float traceLength = (secondaryVaultStartLocation.Z - vaultHit.ImpactPoint.Z);
                                if (!initialSwitch)
                                {
                                    initialTraceHeight = traceLength;
                                    initialSwitch = true;
                                }
                                previousTraceHeight = currentTraceHeight;
                                currentTraceHeight = traceLength;
                                if (!(FMath::IsNearlyEqual(currentTraceHeight, initialTraceHeight, 20.0f)))
                                {
                                    if (FMath::IsNearlyEqual(previousTraceHeight, currentTraceHeight, 3.0f))
                                    {
                                        FVector downTracePoint = vaultHit.Location;
                                        downTracePoint.Z = vaultHit.ImpactPoint.Z;
                                        //UPrimitiveComponent* hitComponent = hit.Component;

                                        FVector calculationVector = FVector::ZeroVector;
                                        calculationVector.Z = (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2);
                                        downTracePoint += calculationVector;
                                        startLocation = downTracePoint;
                                        startLocation.Z += (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
                                        endLocation = downTracePoint;
                                        endLocation.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());

                                        //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
                                        if (!GetWorld()->SweepSingleByChannel(vaultHit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()), TraceParams))
                                        {
                                            forwardImpactNormal.X -= 1;
                                            forwardImpactNormal.Y -= 1;
                                            localTargetTransform = FTransform(UKismetMathLibrary::MakeRotFromX(forwardImpactNormal), downTracePoint);
                                            float height = (localTargetTransform.GetLocation() - GetActorLocation()).Z;
                                            bIsVaulting = true;
                                            Vault(height, localTargetTransform);
                                            vaultFailed = false;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        if (vaultFailed)
                        {
                            FVector downTracePoint = hit.Location;
                            downTracePoint.Z = hit.ImpactPoint.Z;
                            //UPrimitiveComponent* hitComponent = hit.Component;

                            FVector calculationVector = FVector::ZeroVector;
                            calculationVector.Z = (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2);
                            downTracePoint += calculationVector;
                            startLocation = downTracePoint;
                            startLocation.Z += (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
                            endLocation = downTracePoint;
                            endLocation.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());

                            //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
                            if (!GetWorld()->SweepSingleByChannel(hit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()), TraceParams))
                            {
                                forwardImpactNormal.X -= 1;
                                forwardImpactNormal.Y -= 1;
                                localTargetTransform = FTransform(UKismetMathLibrary::MakeRotFromX(forwardImpactNormal), downTracePoint);
                                float height = (localTargetTransform.GetLocation() - GetActorLocation()).Z;
                                bIsVaulting = true;
                                Vault(height, localTargetTransform);
                            }
                        }
                    }
                }
            }
        }
    }
}

void ASCharacter::CheckAngle()
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);
    float length = 0;

    FVector capsuleHeight = GetCapsuleComponent()->GetComponentLocation();
    capsuleHeight.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 1);
    FVector originLocation = capsuleHeight;
    FVector traceStartLocation = (originLocation + (UKismetMathLibrary::GetForwardVector(GetCapsuleComponent()->GetComponentRotation()) * 10));
    FVector traceEndLocation = traceStartLocation;
    traceStartLocation.Z += 100;
    traceEndLocation.Z = 0;
    if (GetWorld()->LineTraceSingleByChannel(angleHit, traceStartLocation, traceEndLocation, ECC_WorldStatic, TraceParams))
    {
        length = (traceStartLocation.Z - angleHit.ImpactPoint.Z - 1);
        if (length > 200)
        {
            length = 0;
        }
        else if (length >= 100)
        {
            length -= 100;
        }
        else if (length < 10)
        {
            length = 100 - length;
        }
    }
    floorAngle = UKismetMathLibrary::Atan(length/10);
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("%f"),length), true);
}

void ASCharacter::Vault(float height, FTransform targetTransform)
{
    VaultStartLocation = GetActorTransform();
    VaultEndLocation = targetTransform;
    movementState = VE_Vault;
    UpdateMovementSpeed();
    meshComp->GetAnimInstance()->Montage_Play(vaultMontage, 1.0f);
    vaultTimeline.PlayFromStart();
}

// Function that determines the player's maximum speed, based on whether they're crouching, sprinting or neither
void ASCharacter::UpdateMovementSpeed()
{
    if (movementState == VE_Crouch)
    {
        GetCharacterMovement()->MaxWalkSpeed = crouchMovementSpeed;
        currentWeapon->bCanFire = true;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->GroundFriction = 8.0f;
    }
    else if (movementState == VE_Sprint)
    {
        GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
        currentWeapon->bCanFire = false;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->GroundFriction = 8.0f;
    }
    else if (movementState == VE_Walk)
    {
        GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
        currentWeapon->bCanFire = true;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->GroundFriction = 8.0f;
    }
    else if (movementState == VE_Slide)
    {
        GetCharacterMovement()->MaxWalkSpeed = slideSpeed;
        currentWeapon->bCanFire = false;
        GetCharacterMovement()->MaxAcceleration = 200.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 200.0f;
        GetCharacterMovement()->GroundFriction = 1.0f;
    }
    else if (movementState == VE_Vault)
    {
        currentWeapon->bCanFire = true;

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
    if (currentWeapon)
    {
        currentWeapon->StartFire();
    }
}

// Passing player inputs to SWeaponBase
void ASCharacter::StopFire()
{
    if (currentWeapon)
    {
        currentWeapon->StopFire();
    }
}

// Passing player inputs to SWeaponBase
void ASCharacter::Reload()
{
    if (currentWeapon)
    {
        currentWeapon->Reload();
    }
}

void ASCharacter::StartADS()
{
    bWantsToAim = true;
}

void ASCharacter::StopADS()
{
    bWantsToAim = false;
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Crouching
	// Sets the new Target Half Height based on whether the player is crouching or standing
	float targetHalfHeight = (movementState == VE_Crouch || movementState == VE_Slide)? finalCapsuleHalfHeight : defaultCapsuleHalfHeight;
	// Interpolates between the current height and the target height
	float newHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), targetHalfHeight, DeltaTime, crouchSpeed);
	// Sets the half height of the capsule component to the new interpolated half height
	GetCapsuleComponent()->SetCapsuleHalfHeight(newHalfHeight);

    // FOV adjustments
    float targetFOV = (movementState == VE_Sprint || movementState == VE_Slide)? speedFOV : defaultFOV;
    //Interpolates between current fov and target fov
    float newFOV = FMath::FInterpTo(cameraComp->FieldOfView, targetFOV, DeltaTime, fovChangeSpeed);
    // Sets the new camera FOV
    cameraComp->SetFieldOfView(newFOV);

    if (bWantsToAim == true && movementState != VE_Sprint && movementState != VE_Slide)
    {
        bIsAiming = true;
    }
    else
    {
        bIsAiming = false;
    }

    if (GetCharacterMovement()->IsMovingOnGround() && !bPerformedSlide)
    {
        if(bWantsToSlide)
        {
            StartSlide();
            bWantsToSlide = false;
        }
    }

    CheckVault();

    vaultTimeline.TickTimeline(DeltaTime);

    CheckAngle();
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
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::StopCrouch);
	
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

    // Aiming Down Sights
    PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ASCharacter::StartADS);
    PlayerInputComponent->BindAction("ADS", IE_Released, this, &ASCharacter::StopADS);
}
