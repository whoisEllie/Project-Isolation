// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacter.h"
#include <string>
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "AmmoPickup.h"
#include "FPSCharacterController.h"
#include "InteractInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WeaponBase.h"
#include "WeaponPickup.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/AudioComponent.h"
#include "Isolation/Isolation.h"

// Sets default values
AFPSCharacter::AFPSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    // Spawning the spring arm component
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->SetupAttachment(RootComponent);
    
    // Spawning the FPS hands mesh component and attaching it to the spring arm component
    HandsMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    HandsMeshComp->CastShadow = false;
    HandsMeshComp->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);
    
    // Spawning the camera atop the FPS hands mesh
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->AttachToComponent(HandsMeshComp, FAttachmentTransformRules::KeepRelativeTransform, "CameraSocket");

    // Spawning the footstep sound component
    FootstepAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudioComp"));
    FootstepAudioComp->SetupAttachment(RootComponent);
    FootstepAudioComp->bAutoActivate = false;
    
    DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
    //DefaultSpringArmHeight = SpringArmComp->GetComponentLocation(); // Setting the default location of the spring arm
    bIsPrimary = true;

    QueryParams.bReturnPhysicalMaterial = true;
}

void AFPSCharacter::TimelineProgress(float Value)
{    
    const FVector NewLocation = FMath::Lerp(VaultStartLocation.GetLocation(), VaultEndLocation.GetLocation(), Value);
    SetActorLocation(NewLocation);
    if (Value == 1)
    {
        bIsVaulting = false;
        if (bHoldingSprint)
        {
            UpdateMovementValues(EMovementState::State_Sprint);
        }
    }
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (IsValid(HUDWidget))
    {
        PlayerHudWidget = Cast<USHUDWidget>(CreateWidget(GetWorld(), HUDWidget));
        if (PlayerHudWidget != nullptr)
        {
            PlayerHudWidget->AddToViewport();
        }
    }
	
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    BaseFOV = CameraComp->FieldOfView;

    if (CurveFloat)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Bound timelineprogress"));
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
        VaultTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
    }
}

void AFPSCharacter::WorldInteract()
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);
    const FVector CameraLocation = CameraComp->GetComponentLocation();
    const FRotator CameraRotation = CameraComp->GetComponentRotation();
    const FVector TraceDirection = CameraRotation.Vector();
    const FVector TraceEndLocation = CameraLocation + (TraceDirection * InteractDistance);

    if (GetWorld()->LineTraceSingleByChannel(InteractionHit, CameraLocation, TraceEndLocation, ECC_WorldStatic, TraceParams))
    {
        if(InteractionHit.GetActor()->GetClass()->ImplementsInterface(USInteractInterface::StaticClass()))
        {
            Cast<ISInteractInterface>(InteractionHit.GetActor())->Interact();
        }
    }
}

// Performing logic for the interaction indicator at the center of the screen
void AFPSCharacter::InteractionIndicator()
{
    bCanInteract = false;
    
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);
    
    const FVector CameraLocation = CameraComp->GetComponentLocation();
    const FRotator CameraRotation = CameraComp->GetComponentRotation();
    const FVector TraceDirection = CameraRotation.Vector();
    const FVector TraceEndLocation = CameraLocation + (TraceDirection * InteractDistance);

    if (GetWorld()->LineTraceSingleByChannel(InteractionHit, CameraLocation, TraceEndLocation, ECC_WorldStatic, TraceParams))
    {        
        if(InteractionHit.GetActor()->GetClass()->ImplementsInterface(USInteractInterface::StaticClass()))
        {
            bCanInteract = true;
            const ASInteractionActor* InteractionActor = Cast<ASInteractionActor>(InteractionHit.GetActor());
            AAmmoPickup* AmmoPickup = Cast<AAmmoPickup>(InteractionHit.GetActor());
            if (InteractionActor)
            {
                InteractText = InteractionActor->PopupDescription;
            }
            else if (AmmoPickup)
            {
                InteractText = AmmoPickup->GetPickupName();
            }
            else
            {
                InteractText = FText::GetEmpty();
            }
            
            const ASWeaponPickup* InteractedPickup = Cast<ASWeaponPickup>(InteractionHit.GetActor());
            if (InteractedPickup)
            {
                bInteractionIsWeapon = true;
                InteractText = InteractedPickup->WeaponName;
            }
            else
            {
                bInteractionIsWeapon = false;
            }
        }
    }
}

// Playing footstep sounds on FootstepAudioComp, called from animnotifies
void AFPSCharacter::FootstepSounds()
{
    const FVector TraceStart = GetActorLocation();
    FVector TraceEnd = TraceStart;
    TraceEnd.Z -= 100.0f;
    
    FootstepAudioComp->SetIntParameter(FName("floor"), 0);
    if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, FOOTSTEP_TRACE, QueryParams))
    {
        FootstepAudioComp->SetIntParameter(FName("floor"), SurfaceMaterialArray.Find(Hit.PhysMaterial.Get()));
        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), TraceStart, Hit.Location, FColor::Red, false, 10.0f, 0.0f, 2.0f);
        }
    }
    else if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 10.0f, 0.0f, 2.0f);
    }

    FootstepAudioComp->Play();
}

// Swapping weapons with the scroll wheel
void AFPSCharacter::ScrollWeapon()
{
    if (bIsPrimary)
    {
        SwapToSecondary();
    }
    else
    {
        SwapToPrimary();
    }
}

// Built in UE function for moving forward/back
void AFPSCharacter::MoveForward(float Value)
{
    ForwardMovement = Value;
	AddMovementInput(GetActorForwardVector() * Value);
}

// Built in UE function for moving left/right
void AFPSCharacter::MoveRight(float Value)
{
    RightMovement = Value;
	AddMovementInput(GetActorRightVector() * Value);
}

// Built in UE function for looking up/down
void AFPSCharacter::LookUp(float Value)
{
    MouseX = Value;
	AddControllerPitchInput(Value);
    // checking mouse movement for recoil compensation logic
    if (Value != 0.0f && CurrentWeapon)
    {
        CurrentWeapon->bShouldRecover = false;
        CurrentWeapon->RecoilRecoveryTimeline.Stop();
    }
}

// Built in UE function for looking left/right
void AFPSCharacter::LookRight(float Value)
{
    MouseY = Value;
	AddControllerYawInput(Value);
    // checking mouse movement for recoil compensation logic
    if (Value != 0.0f && CurrentWeapon)
    {
        CurrentWeapon->bShouldRecover = false;
        CurrentWeapon->RecoilRecoveryTimeline.Stop();
    }
}

// Custom crouch function
void AFPSCharacter::StartCrouch()
{
    bHoldingCrouch = true;
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        if (MovementState == EMovementState::State_Crouch)
        {
            StopCrouch(false);
            //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Ending Crouch", true);
        }
        else if (MovementState == EMovementState::State_Sprint && !bPerformedSlide)
        {
            StartSlide();
        }
        else
        {
            UpdateMovementValues(EMovementState::State_Crouch);
        }
    }
    else if (!bPerformedSlide)
    {
        bWantsToSlide = true;
    }
}

void AFPSCharacter::ReleaseCrouch()
{
    bHoldingCrouch = false;
    bPerformedSlide = false;
    if (MovementState == EMovementState::State_Slide)
    {
        StopSlide();
    }
}

void AFPSCharacter::StopCrouch(const bool bToSprint)
{
    if ((MovementState == EMovementState::State_Crouch || MovementState == EMovementState::State_Slide) && HasSpaceToStandUp())
    {
        if (bToSprint)
        {
            UpdateMovementValues(EMovementState::State_Sprint);
        }
        else
        {
            UpdateMovementValues(EMovementState::State_Walk);
        }
    }
}

// Starting to sprint (IE_Pressed)
void AFPSCharacter::StartSprint()
{
    if (!HasSpaceToStandUp())
    {
        return;
    }
    bHoldingSprint = true;
    bPerformedSlide = false;
    // Updates the sprint speed
    UpdateMovementValues(EMovementState::State_Sprint);
}

// Stopping to sprint (IE_Released)
void AFPSCharacter::StopSprint()
{
    if (!HasSpaceToStandUp())
    {
        return;
    }
    
    if (MovementState == EMovementState::State_Slide && bHoldingCrouch)
    {
        UpdateMovementValues(EMovementState::State_Crouch);
    }
    else
    {
        UpdateMovementValues(EMovementState::State_Walk);
    }
    
    bHoldingSprint = false;
}

void AFPSCharacter::StartSlide()
{
    bPerformedSlide = true;
    UpdateMovementValues(EMovementState::State_Slide);
    GetWorldTimerManager().SetTimer(SlideStop, this, &AFPSCharacter::ReleaseCrouch, SlideTime, false, SlideTime);
}

void AFPSCharacter::StopSlide()
{
    if (MovementState == EMovementState::State_Slide && FloorAngle > -15.0f)
    {
        if (!HasSpaceToStandUp())
        {
            UpdateMovementValues(EMovementState::State_Crouch);
        }
        else if (bHoldingSprint)
        {
            StopCrouch(true);
        }
        else if (bHoldingCrouch)
        {
            UpdateMovementValues(EMovementState::State_Crouch);
        }
        else
        {
            UpdateMovementValues(EMovementState::State_Walk);
        }
        bPerformedSlide = false;
        GetWorldTimerManager().ClearTimer(SlideStop);
    }
    else if (FloorAngle < -15.0f)
    {
        GetWorldTimerManager().SetTimer(SlideStop, this, &AFPSCharacter::ReleaseCrouch, 0.1f, false, 0.1f);
    }
}

void AFPSCharacter::CheckVault()
{    
    float ForwardVelocity = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
    if (!(ForwardVelocity > 0 && !bIsVaulting && GetCharacterMovement()->IsFalling())) return;

    // store these for future use.
    FVector ColliderLocation = GetCapsuleComponent()->GetComponentLocation();
    FRotator ColliderRotation = GetCapsuleComponent()->GetComponentRotation();


    FVector StartLocation = ColliderLocation;
    FVector EndLocation = ColliderLocation + (UKismetMathLibrary::GetForwardVector(ColliderRotation) * 75);
    if (bDrawDebug)
    {
        DrawDebugCapsule(GetWorld(), StartLocation, 50, 30, FQuat::Identity, FColor::Red);
    }

    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);

    if (!GetWorld()->SweepSingleByChannel(Hit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeCapsule(30, 50), TraceParams)) return;
    if (!Hit.bBlockingHit) return;
    
    FVector ForwardImpactPoint = Hit.ImpactPoint;
    FVector ForwardImpactNormal = Hit.ImpactNormal;
    FVector CapsuleLocation = ForwardImpactPoint;
    CapsuleLocation.Z = ColliderLocation.Z;
    CapsuleLocation += (ForwardImpactNormal * -15);

    StartLocation = CapsuleLocation;
    StartLocation.Z += 100;
    EndLocation = CapsuleLocation;

    if (!GetWorld()->SweepSingleByChannel(Hit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(1), TraceParams)) return;
    if (!GetCharacterMovement()->IsWalkable(Hit)) return;
    
    FVector SecondaryVaultStartLocation = Hit.ImpactPoint;
    SecondaryVaultStartLocation.Z += 5;
    FVector SecondaryVaultEndLocation = SecondaryVaultStartLocation;
    SecondaryVaultEndLocation.Z = 0;

    if (bDrawDebug)
    {
        DrawDebugSphere(GetWorld(), SecondaryVaultStartLocation, 10, 8, FColor::Orange);
    }

    float InitialTraceHeight = 0;
    float PreviousTraceHeight = 0;
    float CurrentTraceHeight = 0;
    bool bInitialSwitch = false;
    bool bVaultFailed = true;

    int i;

    FVector ForwardAddition = UKismetMathLibrary::GetForwardVector(ColliderRotation) * 5;
    float CalculationHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2;
    float ScaledCapsuleWithoutHemisphere = GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();
    
    for (i = 0; i <= VaultTraceAmount; i++)
    {
        SecondaryVaultStartLocation += ForwardAddition;
        SecondaryVaultEndLocation += ForwardAddition;
        bVaultFailed = true;
        if (!GetWorld()->LineTraceSingleByChannel(VaultHit, SecondaryVaultStartLocation, SecondaryVaultEndLocation, ECC_WorldStatic, TraceParams)) continue;
        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), SecondaryVaultStartLocation, VaultHit.ImpactPoint, FColor::Red, false, 10.0f, 0.0f, 2.0f);
        }

        float TraceLength = (SecondaryVaultStartLocation.Z - VaultHit.ImpactPoint.Z);
        if (!bInitialSwitch)
        {
            InitialTraceHeight = TraceLength;
            bInitialSwitch = true;
        }

        PreviousTraceHeight = CurrentTraceHeight;
        CurrentTraceHeight = TraceLength;
        if (!(!(FMath::IsNearlyEqual(CurrentTraceHeight, InitialTraceHeight, 20.0f)) && CurrentTraceHeight < MaxVaultHeight)) continue;

        if (!FMath::IsNearlyEqual(PreviousTraceHeight, CurrentTraceHeight, 3.0f)) continue;

        FVector DownTracePoint = VaultHit.Location;
        DownTracePoint.Z = VaultHit.ImpactPoint.Z;
        //UPrimitiveComponent* hitComponent = hit.Component;

        FVector CalculationVector = FVector::ZeroVector;
        CalculationVector.Z = CalculationHeight;
        DownTracePoint += CalculationVector;
        StartLocation = DownTracePoint;
        StartLocation.Z += ScaledCapsuleWithoutHemisphere;
        EndLocation = DownTracePoint;
        EndLocation.Z -= ScaledCapsuleWithoutHemisphere;

        if (bDrawDebug)
        {
            DrawDebugSphere(GetWorld(), StartLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
        }
        if (GetWorld()->SweepSingleByChannel(VaultHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()), TraceParams)) continue;

        ForwardImpactNormal.X -= 1;
        ForwardImpactNormal.Y -= 1;
        LocalTargetTransform = FTransform(UKismetMathLibrary::MakeRotFromX(ForwardImpactNormal), DownTracePoint);
        bIsVaulting = true;
        Vault(LocalTargetTransform);
        bVaultFailed = false;
        break;
    }

    if (!bVaultFailed) return;
    
    FVector DownTracePoint = Hit.Location;
    DownTracePoint.Z = Hit.ImpactPoint.Z;
    //UPrimitiveComponent* hitComponent = hit.Component;

    FVector CalculationVector = FVector::ZeroVector;
    CalculationVector.Z = (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2);
    DownTracePoint += CalculationVector;
    StartLocation = DownTracePoint;
    StartLocation.Z += (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
    EndLocation = DownTracePoint;
    EndLocation.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());

    //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
    if (GetWorld()->SweepSingleByChannel(Hit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()), TraceParams)) return;

    ForwardImpactNormal.X -= 1;
    ForwardImpactNormal.Y -= 1;
    LocalTargetTransform = FTransform(UKismetMathLibrary::MakeRotFromX(ForwardImpactNormal), DownTracePoint);
    bIsVaulting = true;
    
    Vault(LocalTargetTransform);
}

void AFPSCharacter::CheckAngle(float DeltaTime)
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);

    FVector CapsuleHeight = GetCapsuleComponent()->GetComponentLocation();
    CapsuleHeight.Z -= (GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    const FVector AngleStartTrace = CapsuleHeight;
    FVector AngleEndTrace = AngleStartTrace;
    AngleEndTrace.Z -= 50;
    if (GetWorld()->LineTraceSingleByChannel(AngleHit, AngleStartTrace, AngleEndTrace, ECC_WorldStatic, TraceParams))
    {
        FloorVector = AngleHit.ImpactNormal;
        const FRotator FinalRotation = UKismetMathLibrary::MakeRotFromZX(FloorVector, GetActorForwardVector());
        FloorAngle = FinalRotation.Pitch;
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Current floor angle = %f"),FloorAngle), true);
        }
    }
}

bool AFPSCharacter::HasSpaceToStandUp()
{
    FVector CenterVector = GetActorLocation();
    CenterVector.Z += 44;

    const float CollisionCapsuleHeight = DefaultCapsuleHalfHeight - 17.0f;
        
    const FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(30.0f, CollisionCapsuleHeight);

    if (bDrawDebug)
    {
        DrawDebugCapsule(GetWorld(), CenterVector, CollisionCapsuleHeight, 30.0f, FQuat::Identity, FColor::Red, false, 5.0f, 0, 3);
    }
            
    if (GetWorld()->SweepSingleByChannel(Hit, CenterVector, CenterVector, FQuat::Identity, STAND_UP_CHECK_COLLISION, CollisionCapsule))
    {
        /* confetti or smth idk */
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "SweepSingleByChannel returned true", true);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, Hit.Actor->GetName());
        }
        return false;
    }

    return true;
}

void AFPSCharacter::Vault(const FTransform TargetTransform)
{
    VaultStartLocation = GetActorTransform();
    VaultEndLocation = TargetTransform;
    UpdateMovementValues(EMovementState::State_Vault);
    HandsMeshComp->GetAnimInstance()->Montage_Play(VaultMontage, 1.0f);
    VaultTimeline.PlayFromStart();
}

// Function that determines the player's maximum speed and other related variables based on movement state
void AFPSCharacter::UpdateMovementValues(const EMovementState NewMovementState)
{
    bIsSprinting = false;
    bIsCrouching = false;

    MovementState = NewMovementState;

    switch (MovementState)
    {
        case EMovementState::State_Crouch:
            bIsCrouching = true;
            GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
            if (CurrentWeapon)
            {
                CurrentWeapon->bCanFire = true;
            }
            GetCharacterMovement()->MaxAcceleration = 2048.0f;
            GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
            GetCharacterMovement()->GroundFriction = 8.0f;
            break;

        case EMovementState::State_Sprint:
            bIsSprinting = true;
            GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
            if (CurrentWeapon)
            {
                CurrentWeapon->bCanFire = false;
            }
            GetCharacterMovement()->MaxAcceleration = 2048.0f;
            GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
            GetCharacterMovement()->GroundFriction = 8.0f;
            break;

        case EMovementState::State_Walk:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            if (CurrentWeapon)
            {
                CurrentWeapon->bCanFire = true;
            }
            GetCharacterMovement()->MaxAcceleration = 2048.0f;
            GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
            GetCharacterMovement()->GroundFriction = 8.0f;
            break;

        case EMovementState::State_Slide:
            GetCharacterMovement()->MaxWalkSpeed = SlideSpeed;
            if (CurrentWeapon)
            {
                CurrentWeapon->bCanFire = false;
            }
            GetCharacterMovement()->MaxAcceleration = 200.0f;
            GetCharacterMovement()->BrakingDecelerationWalking = 200.0f;
            GetCharacterMovement()->GroundFriction = 1.0f;
            break;

        case EMovementState::State_Vault:
            if (CurrentWeapon)
            {
                CurrentWeapon->bCanFire = true;
            }
            break;

        default:
            break;
    }
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void AFPSCharacter::UpdateWeapon(const TSubclassOf<ASWeaponBase> NewWeapon, const bool bSpawnPickup,
                                 FWeaponDataStruct* CurrentDataStruct, const bool bStatic, const FTransform PickupTransform)
{
    // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (CurrentWeapon)
    {
        if (bSpawnPickup)
        {
            // Calculating the location where to spawn the new weapon in front of the player
            const FVector TraceStart = CameraComp->GetComponentLocation();
            const FRotator TraceStartRotation = CameraComp->GetComponentRotation();
            const FVector TraceDirection = TraceStartRotation.Vector();
            const FVector TraceEnd = TraceStart + (TraceDirection * WeaponSpawnDistance);

            // Spawning the new pickup
            ASWeaponPickup* NewPickup = GetWorld()->SpawnActor<ASWeaponPickup>(CurrentWeapon->WeaponData->PickupReference, TraceEnd, FRotator::ZeroRotator, SpawnParameters);
            if (bStatic)
            {
                NewPickup->MainMesh->SetSimulatePhysics(false);
                NewPickup->SetActorTransform(PickupTransform);
            }
            // Applying the current weapon data to the pickup
            NewPickup->bStatic = bStatic;
            NewPickup->bRuntimeSpawned = true;
            NewPickup->WeaponReference = CurrentWeapon->GetClass();
            NewPickup->DataStruct = *CurrentDataStruct;
            NewPickup->AttachmentArray = CurrentDataStruct->WeaponAttachments;
            NewPickup->SpawnAttachmentMesh();
        }
        
        // Destroys the current weapon, if it exists
        CurrentWeapon->K2_DestroyActor();
    }
    // Spawns the new weapon and sets the player as it's owner
    CurrentWeapon = GetWorld()->SpawnActor<ASWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (CurrentWeapon)
    {
        CurrentWeapon->SetOwner(this);
        CurrentWeapon->AttachToComponent(HandsMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeapon->WeaponData->WeaponAttachmentSocketName);
    }
}

FText AFPSCharacter::GetCurrentWeaponLoadedAmmo() const
{
    return bIsPrimary? FText::AsNumber(PrimaryWeaponCacheMap.ClipSize) : FText::AsNumber(SecondaryWeaponCacheMap.ClipSize);
}

FText AFPSCharacter::GetCurrentWeaponRemainingAmmo() const
{
    ASCharacterController* CharacterController = Cast<ASCharacterController>(GetController());

    if (CharacterController)
    {
        return bIsPrimary? FText::AsNumber(CharacterController->AmmoMap[PrimaryWeaponCacheMap.AmmoType]) : FText::AsNumber(CharacterController->AmmoMap[SecondaryWeaponCacheMap.AmmoType]);
    }
    else
    {
        return FText::FromString("No Character Controller found");
    }
}


// Spawning and equipping the primary weapon 
void AFPSCharacter::SwapToPrimary()
{
    if (PrimaryWeapon && !bIsPrimary && !CurrentWeapon->bIsReloading)
    {
        // Calling UpdateWeapon with relevant variables
        UpdateWeapon(PrimaryWeapon, false, &SecondaryWeaponCacheMap, false, FTransform::Identity);

        // Spawning attachments based on the local cache map
        CurrentWeapon->SpawnAttachments(PrimaryWeaponCacheMap.WeaponAttachments);

        if (CurrentWeapon)
        {
            if (CurrentWeapon->WeaponData->WeaponEquip)
            {
                HandsMeshComp->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponEquip, 1.0f);
            }
        }

        if (bDrawDebug)
        {
            for (const FName Attachment: PrimaryWeaponCacheMap.WeaponAttachments)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Attachment.ToString());
            }
        }
        bIsPrimary = true;
    }
}

// Spawning and equipping the secondary weapon
void AFPSCharacter::SwapToSecondary()
{
    if (SecondaryWeapon && bIsPrimary && !CurrentWeapon->bIsReloading)
    {
        // Calling UpdateWeapon with relevant variables
        UpdateWeapon(SecondaryWeapon, false, &PrimaryWeaponCacheMap, false, FTransform::Identity);

        // Spawning attachments based on the local cache map
        CurrentWeapon->SpawnAttachments(SecondaryWeaponCacheMap.WeaponAttachments);

        if (CurrentWeapon)
        {
            if (CurrentWeapon->WeaponData->WeaponEquip)
            {
                HandsMeshComp->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponEquip, 1.0f);
            }
        }

        if (bDrawDebug)
        {
            for (const FName Attachment: SecondaryWeaponCacheMap.WeaponAttachments)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Attachment.ToString());
            }
        }
        bIsPrimary = false;
    }
}

// Passing player inputs to WeaponBase
void AFPSCharacter::StartFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

// Passing player inputs to WeaponBase
void AFPSCharacter::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

// Passing player inputs to WeaponBase
void AFPSCharacter::Reload()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}

void AFPSCharacter::StartAds()
{
    bWantsToAim = true;
}

void AFPSCharacter::StopAds()
{
    bWantsToAim = false;
}

// Called every frame
void AFPSCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

    // Timeline tick
    VaultTimeline.TickTimeline(DeltaTime);

	// Crouching
	// Sets the new Target Half Height based on whether the player is crouching or standing
	const float TargetHalfHeight = (MovementState == EMovementState::State_Crouch || MovementState == EMovementState::State_Slide)? CrouchedCapsuleHalfHeight : DefaultCapsuleHalfHeight;
    const float SpringArmTargetOffset = (MovementState == EMovementState::State_Crouch || MovementState == EMovementState::State_Slide)? CrouchedSpringArmHeightDelta : 0.0f;
    // Interpolates between the current height and the target height
	const float NewHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetHalfHeight, DeltaTime, CrouchSpeed);
    const float NewLocation = FMath::FInterpTo(CurrentSpringArmOffset, SpringArmTargetOffset, DeltaTime, CrouchSpeed);
    CurrentSpringArmOffset = NewLocation;
	// Sets the half height of the capsule component to the new interpolated half height
	GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);
    FVector NewSpringArmLocation = SpringArmComp->GetRelativeLocation();
    NewSpringArmLocation.Z = NewLocation;
    SpringArmComp->SetRelativeLocation(NewSpringArmLocation);
    
    // FOV adjustments
    float TargetFOV = ((MovementState == EMovementState::State_Sprint || MovementState == EMovementState::State_Slide) && GetVelocity().Size() > WalkSpeed)? (BaseFOV + FOVChangeAmount) : BaseFOV;
    if (CurrentWeapon)
    {
        if (bIsAiming && CurrentWeapon->WeaponData->bAimingFOV && !CurrentWeapon->bIsReloading)
        {
            TargetFOV = BaseFOV - CurrentWeapon->WeaponData->AimingFOVChange;
            FOVChangeSpeed = 6;
        }
    }
    //Interpolates between current fov and target fov
    const float InFieldOfView = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, FOVChangeSpeed);
    // Sets the new camera FOV
    CameraComp->SetFieldOfView(InFieldOfView);

    // Continuous aiming check (so that you don't have to re-press the ADS button every time you jump/sprint/reload/etc)
    if (bWantsToAim == true && MovementState != EMovementState::State_Sprint && MovementState != EMovementState::State_Slide)
    {
        bIsAiming = true;
    }
    else
    {
        bIsAiming = false;
    }

    // 
    if (GetCharacterMovement()->IsMovingOnGround() && !bPerformedSlide && bWantsToSlide)
    {
        StartSlide();
        bWantsToSlide = false;
    }

    CheckVault();

    CheckAngle(DeltaTime);


    if (bDrawDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(SecondaryWeaponCacheMap.ClipSize));
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(SecondaryWeaponCacheMap.ClipCapacity));
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(SecondaryWeaponCacheMap.WeaponHealth));
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("Secondary"));

        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(PrimaryWeaponCacheMap.ClipSize));
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(PrimaryWeaponCacheMap.ClipCapacity));
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(PrimaryWeaponCacheMap.WeaponHealth));
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("Primary"));
    }
    
    InteractionIndicator();


    //Updating material parameters for    

    if (CurrentWeapon)
    {
        if (CurrentWeapon->WeaponData)
        {
            if (bIsAiming)
            {
                ScopeBlend = FMath::FInterpConstantTo(ScopeBlend, 1, DeltaTime, 8.0f);
                UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScopeOpacityParameterCollection, OpacityParameterName,ScopeBlend);
            }
            else
            {
                ScopeBlend = FMath::FInterpConstantTo(ScopeBlend, 0, DeltaTime, 8.0f);
                UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScopeOpacityParameterCollection, OpacityParameterName,ScopeBlend);
            }
        }
    }
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Move forward/back + left/right inputs
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);
	
	// Look up/down + left/right
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AFPSCharacter::LookRight);
	
	// Crouching
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPSCharacter::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AFPSCharacter::ReleaseCrouch);
	
	// Sprinting
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFPSCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AFPSCharacter::StopSprint);
	
	// Jumping
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSCharacter::Jump);

    // Weapon swap
    PlayerInputComponent->BindAction("PrimaryWeapon", IE_Pressed, this, &AFPSCharacter::SwapToPrimary);
    PlayerInputComponent->BindAction("SecondaryWeapon", IE_Pressed, this, &AFPSCharacter::SwapToSecondary);

    // Firing
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::StopFire);

    // Reloading
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::Reload);

    // Aiming Down Sights
    PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AFPSCharacter::StartAds);
    PlayerInputComponent->BindAction("ADS", IE_Released, this, &AFPSCharacter::StopAds);

    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFPSCharacter::WorldInteract);

    PlayerInputComponent->BindAction("ScrollUp", IE_Pressed, this, &AFPSCharacter::ScrollWeapon);
}
