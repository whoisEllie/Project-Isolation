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
#include "SInteractInterface.h"
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
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->SetupAttachment(RootComponent);
    
    // Spawning the FPS hands mesh component and attaching it to the spring arm component
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("meshComp"));
    MeshComp->CastShadow = false;
    MeshComp->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);
    
    // Spawning the camera atop the FPS hands mesh
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
    CameraComp->SetupAttachment(MeshComp, "cameraSocket");
    
    CrouchSpeed = 10.0f; // the speed at which the player crouches, can be overridden in BP_Character
    DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
    FinalCapsuleHalfHeight = 44.0f; // the desired final crouch height, can be overridden in BP_Character
}

void ASCharacter::TimelineProgress(float value)
{
    FVector newActorLocation = FMath::Lerp(VaultStartLocation.GetLocation(), VaultEndLocation.GetLocation(), value);
    SetActorLocation(newActorLocation);
    if (value == 1)
    {
        bIsVaulting = false;
    }
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    UpdateWeapon(PrimaryWeapon);
    DefaultFOV = CameraComp->FieldOfView;
    SpeedFOV = DefaultFOV + FOVChangeAmount;

    if (CurveFloat)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
        VaultTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
    }
}

void ASCharacter::WorldInteract()
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);
    const FVector CameraLocation = CameraComp->GetComponentLocation();
    const FRotator CameraRotation = CameraComp->GetComponentRotation();
    const FVector TraceDirection = CameraRotation.Vector();
    const FVector TraceEndLocation = CameraLocation + (TraceDirection * 10000.0f);

    if(GetWorld()->LineTraceSingleByChannel(InteractionHit, CameraLocation, TraceEndLocation, ECC_WorldStatic, TraceParams))
    {
        if(InteractionHit.GetActor()->GetClass()->ImplementsInterface(USInteractInterface::StaticClass()))
        {
            Cast<ISInteractInterface>(InteractionHit.GetActor())->Interact();
        }
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
        if (MovementState == VE_Crouch)
        {
            EndCrouch(false);
            //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Ending Crouch", true);
        }
        else if (MovementState == VE_Sprint && !bPerformedSlide)
        {
            StartSlide();
        }
        else
        {
            MovementState = VE_Crouch;
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
    if (MovementState == VE_Slide)
    {
        StopSlide();
        bPerformedSlide = false;
    }
}

void ASCharacter::EndCrouch(bool ToSprint)
{
    if (MovementState == VE_Crouch || MovementState == VE_Slide)
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
        if (ToSprint)
        {
            MovementState = VE_Sprint;
        }
        else
        {
            MovementState = VE_Walk;
        }
        UpdateMovementSpeed();
    }
}

// Starting to sprint (IE_Pressed)
void ASCharacter::StartSprint()
{
    bHoldingSprint = true;
    // Updates the sprint speed
    MovementState = VE_Sprint;
    UpdateMovementSpeed();
}

// Stopping to sprint (IE_Released)
void ASCharacter::StopSprint()
{
    if (MovementState == VE_Slide && bHoldingCrouch)
    {
        MovementState = VE_Crouch;
    }
    else
    {
        MovementState = VE_Walk;
    }
    
    bHoldingSprint = false;
    UpdateMovementSpeed();
}

void ASCharacter::StartSlide()
{
    MovementState = VE_Slide;
    bPerformedSlide = true;
    UpdateMovementSpeed();
    GetWorldTimerManager().SetTimer(SlideStop, this, &ASCharacter::StopSlide, SlideTime, false, SlideTime);
}

void ASCharacter::StopSlide()
{
    if (MovementState == VE_Slide && FloorAngle > -15.0f)
    {
        if (bHoldingSprint)
        {
            EndCrouch(true);
        }
        else if (bHoldingCrouch)
        {
            MovementState = VE_Crouch;
        }
        else
        {
            MovementState = VE_Walk;
        }
        bPerformedSlide = false;
        GetWorldTimerManager().ClearTimer(SlideStop);
        UpdateMovementSpeed();
    }
    else if (FloorAngle < -15.0f)
    {
        GetWorldTimerManager().SetTimer(SlideStop, this, &ASCharacter::StopSlide, 0.1f, false, 0.1f);
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

        if (GetWorld()->SweepSingleByChannel(Hit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeCapsule(30, 50), TraceParams))
        {
            if (Hit.bBlockingHit)
            {
                FVector forwardImpactPoint = Hit.ImpactPoint;
                FVector forwardImpactNormal = Hit.ImpactNormal;
                FVector capsuleLocation = forwardImpactPoint;
                capsuleLocation.Z = (GetCapsuleComponent()->GetComponentLocation().Z);
                capsuleLocation += (forwardImpactNormal * -15);
                startLocation = capsuleLocation;
                startLocation.Z += 100;
                endLocation = capsuleLocation;
                //DrawDebugSphere(GetWorld(), startLocation, 1, 4, FColor::Blue);
                if (GetWorld()->SweepSingleByChannel(Hit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(1), TraceParams))
                {
                    if (GetCharacterMovement()->IsWalkable(Hit))
                    {
                        FVector secondaryVaultStartLocation = Hit.ImpactPoint;
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
                        for (i = 0; i <= VaultTraceAmount; i++)
                        {
                            secondaryVaultStartLocation += (UKismetMathLibrary::GetForwardVector(cacheRotator) * 5);
                            secondaryVaultEndLocation += (UKismetMathLibrary::GetForwardVector(cacheRotator) * 5);
                            vaultFailed = true;
                            if(GetWorld()->LineTraceSingleByChannel(VaultHit, secondaryVaultStartLocation, secondaryVaultEndLocation, ECC_WorldStatic, TraceParams))
                            {
                                if (bDrawDebug)
                                {
                                    DrawDebugLine(GetWorld(), secondaryVaultStartLocation, VaultHit.ImpactPoint, FColor::Red, false, 10.0f, 0.0f, 2.0f);
                                }

                                float traceLength = (secondaryVaultStartLocation.Z - VaultHit.ImpactPoint.Z);
                                if (!initialSwitch)
                                {
                                    initialTraceHeight = traceLength;
                                    initialSwitch = true;
                                }
                                previousTraceHeight = currentTraceHeight;
                                currentTraceHeight = traceLength;
                                if (!(FMath::IsNearlyEqual(currentTraceHeight, initialTraceHeight, 20.0f)) && currentTraceHeight < MaxVaultHeight)
                                {
                                    if (FMath::IsNearlyEqual(previousTraceHeight, currentTraceHeight, 3.0f))
                                    {
                                        FVector downTracePoint = VaultHit.Location;
                                        downTracePoint.Z = VaultHit.ImpactPoint.Z;
                                        //UPrimitiveComponent* hitComponent = hit.Component;

                                        FVector calculationVector = FVector::ZeroVector;
                                        calculationVector.Z = (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2);
                                        downTracePoint += calculationVector;
                                        startLocation = downTracePoint;
                                        startLocation.Z += (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
                                        endLocation = downTracePoint;
                                        endLocation.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());

                                        //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
                                        if (!GetWorld()->SweepSingleByChannel(VaultHit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()), TraceParams))
                                        {
                                            forwardImpactNormal.X -= 1;
                                            forwardImpactNormal.Y -= 1;
                                            LocalTargetTransform = FTransform(UKismetMathLibrary::MakeRotFromX(forwardImpactNormal), downTracePoint);
                                            float height = (LocalTargetTransform.GetLocation() - GetActorLocation()).Z;
                                            bIsVaulting = true;
                                            Vault(height, LocalTargetTransform);
                                            vaultFailed = false;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        if (vaultFailed)
                        {
                            FVector downTracePoint = Hit.Location;
                            downTracePoint.Z = Hit.ImpactPoint.Z;
                            //UPrimitiveComponent* hitComponent = hit.Component;

                            FVector calculationVector = FVector::ZeroVector;
                            calculationVector.Z = (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2);
                            downTracePoint += calculationVector;
                            startLocation = downTracePoint;
                            startLocation.Z += (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
                            endLocation = downTracePoint;
                            endLocation.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());

                            //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
                            if (!GetWorld()->SweepSingleByChannel(Hit, startLocation, endLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()), TraceParams))
                            {
                                forwardImpactNormal.X -= 1;
                                forwardImpactNormal.Y -= 1;
                                LocalTargetTransform = FTransform(UKismetMathLibrary::MakeRotFromX(forwardImpactNormal), downTracePoint);
                                float height = (LocalTargetTransform.GetLocation() - GetActorLocation()).Z;
                                bIsVaulting = true;
                                Vault(height, LocalTargetTransform);
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
    FRotator finalRotation = FRotator::ZeroRotator;

    FVector capsuleHeight = GetCapsuleComponent()->GetComponentLocation();
    capsuleHeight.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    FVector angleStartTrace = capsuleHeight;
    FVector angleEndTrace = angleStartTrace;
    angleEndTrace.Z -= 50;
    if (GetWorld()->LineTraceSingleByChannel(AngleHit, angleStartTrace, angleEndTrace, ECC_WorldStatic, TraceParams))
    {
        FloorVector = AngleHit.ImpactNormal;
        finalRotation = UKismetMathLibrary::MakeRotFromZX(FloorVector, GetActorForwardVector());
        FloorAngle = finalRotation.Pitch;
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("%f"),FloorAngle), true);
        }
    }
}

void ASCharacter::Vault(float Height, FTransform TargetTransform)
{
    VaultStartLocation = GetActorTransform();
    VaultEndLocation = TargetTransform;
    MovementState = VE_Vault;
    UpdateMovementSpeed();
    MeshComp->GetAnimInstance()->Montage_Play(VaultMontage, 1.0f);
    VaultTimeline.PlayFromStart();
}

// Function that determines the player's maximum speed, based on whether they're crouching, sprinting or neither
void ASCharacter::UpdateMovementSpeed()
{
    if (MovementState == VE_Crouch)
    {
        GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
        CurrentWeapon->bCanFire = true;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->GroundFriction = 8.0f;
    }
    else if (MovementState == VE_Sprint)
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
        CurrentWeapon->bCanFire = false;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->GroundFriction = 8.0f;
    }
    else if (MovementState == VE_Walk)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        CurrentWeapon->bCanFire = true;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->GroundFriction = 8.0f;
    }
    else if (MovementState == VE_Slide)
    {
        GetCharacterMovement()->MaxWalkSpeed = SlideSpeed;
        CurrentWeapon->bCanFire = false;
        GetCharacterMovement()->MaxAcceleration = 200.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 200.0f;
        GetCharacterMovement()->GroundFriction = 1.0f;
    }
    else if (MovementState == VE_Vault)
    {
        CurrentWeapon->bCanFire = true;

    }
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void ASCharacter::UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon)
{
    // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters spawnParams;
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (CurrentWeapon)
    {
        // Destroys the current weapon, if it exists
        CurrentWeapon->K2_DestroyActor();
    }
    // Spawns the new weapon and sets the player as it's owner
    CurrentWeapon = GetWorld()->SpawnActor<ASWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams);
    if (CurrentWeapon)
    {
        CurrentWeapon->SetOwner(this);
        CurrentWeapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeapon->weaponAttachmentSocketName);
    }   
}

// Lazy solution - read below
void ASCharacter::SwapToPrimary()
{
    UpdateWeapon(PrimaryWeapon);
}

// Lazy solution - read below
void ASCharacter::SwapToSecondary()
{
    UpdateWeapon(SecondaryWeapon);
}

// Passing player inputs to SWeaponBase
void ASCharacter::StartFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

// Passing player inputs to SWeaponBase
void ASCharacter::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

// Passing player inputs to SWeaponBase
void ASCharacter::Reload()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
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
	float targetHalfHeight = (MovementState == VE_Crouch || MovementState == VE_Slide)? FinalCapsuleHalfHeight : DefaultCapsuleHalfHeight;
	// Interpolates between the current height and the target height
	float newHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), targetHalfHeight, DeltaTime, CrouchSpeed);
	// Sets the half height of the capsule component to the new interpolated half height
	GetCapsuleComponent()->SetCapsuleHalfHeight(newHalfHeight);

    // FOV adjustments
    float targetFOV = (MovementState == VE_Sprint || MovementState == VE_Slide)? SpeedFOV : DefaultFOV;
    //Interpolates between current fov and target fov
    float newFOV = FMath::FInterpTo(CameraComp->FieldOfView, targetFOV, DeltaTime, FOVChangeSpeed);
    // Sets the new camera FOV
    CameraComp->SetFieldOfView(newFOV);

    if (bWantsToAim == true && MovementState != VE_Sprint && MovementState != VE_Slide)
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

    VaultTimeline.TickTimeline(DeltaTime);

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

    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASCharacter::WorldInteract);
}
