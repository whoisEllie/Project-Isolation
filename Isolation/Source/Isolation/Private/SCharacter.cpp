// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"

#include <string>

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "SAmmoPickup.h"
#include "SCharacterController.h"
#include "SInteractInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SWeaponBase.h"
#include "SWeaponPickup.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/InputSettings.h"

// Sets default values
ASCharacter::ASCharacter()
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
    
    CrouchSpeed = 10.0f; // the speed at which the player crouches, can be overridden in BP_Character
    DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
    FinalCapsuleHalfHeight = 44.0f; // the desired final crouch height, can be overridden in BP_Character
    bIsPrimary = true;

    QueryParams.bReturnPhysicalMaterial = true;
}

void ASCharacter::TimelineProgress(float value)
{
    const FVector NewLocation = FMath::Lerp(VaultStartLocation.GetLocation(), VaultEndLocation.GetLocation(), value);
    SetActorLocation(NewLocation);
    if (value == 1)
    {
        bIsVaulting = false;
        if (bHoldingSprint)
        {
            UpdateMovementValues(EMovementState::State_Sprint);
        }
    }
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (IsValid(HUDWidget))
    {
        UserWidget = Cast<USHUDWidget>(CreateWidget(GetWorld(), HUDWidget));
        if (UserWidget != nullptr)
        {
            UserWidget->AddToViewport();
        }
    }
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    BaseFOV = CameraComp->FieldOfView;

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
void ASCharacter::InteractionIndicator()
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
            ASAmmoPickup* AmmoPickup = Cast<ASAmmoPickup>(InteractionHit.GetActor());
            if (InteractionActor)
            {
                InteractText = InteractionActor->PopupDescription;
            }
            else if (AmmoPickup)
            {
                InteractText = *AmmoPickup->PickupName.Find(AmmoPickup->AmmoType);
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

void ASCharacter::FootstepSounds()
{
    const FVector TraceStart = GetActorLocation();
    FVector TraceEnd = TraceStart;
    TraceEnd.Z -= 100.0f;
    
    if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_GameTraceChannel3, QueryParams))
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

void ASCharacter::SwapToPrimary()
{
    UpdateWeapon(PrimaryWeapon, false, SecondaryWeaponCacheMap, PrimaryWeaponCacheMap,
    false, FTransform::Identity, true);
}

void ASCharacter::SwapToSecondary()
{
    UpdateWeapon(SecondaryWeapon, false, PrimaryWeaponCacheMap, SecondaryWeaponCacheMap,
    false, FTransform::Identity, false);
}

// Swapping weapons with the scroll wheel
void ASCharacter::ScrollWeapon()
{
    if (bIsPrimary)
    {
        UpdateWeapon(SecondaryWeapon, false, PrimaryWeaponCacheMap, SecondaryWeaponCacheMap,
    false, FTransform::Identity, false);
    }
    else
    {
        UpdateWeapon(PrimaryWeapon, false, SecondaryWeaponCacheMap, PrimaryWeaponCacheMap,
    false, FTransform::Identity, true);
    }
}


// Built in UE function for moving forward/back
void ASCharacter::MoveForward(float value)
{
    ForwardMovement = value;
	AddMovementInput(GetActorForwardVector() * value);
}

// Built in UE function for moving left/right
void ASCharacter::MoveRight(float value)
{
    RightMovement = value;
	AddMovementInput(GetActorRightVector() * value);
}

// Built in UE function for looking up/down
void ASCharacter::LookUp(float value)
{
    MouseX = value;
	AddControllerPitchInput(value);
    // checking mouse movement for recoil compensation logic
    if (value != 0.0f && CurrentWeapon)
    {
        CurrentWeapon->bShouldRecover = false;
        CurrentWeapon->RecoilRecoveryTimeline.Stop();
    }
}

// Built in UE function for looking left/right
void ASCharacter::LookRight(float value)
{
    MouseY = value;
	AddControllerYawInput(value);
    // checking mouse movement for recoil compensation logic
    if (value != 0.0f && CurrentWeapon)
    {
        CurrentWeapon->bShouldRecover = false;
        CurrentWeapon->RecoilRecoveryTimeline.Stop();
    }
}

// Custom crouch function
void ASCharacter::StartCrouch()
{
    bHoldingCrouch = true;
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        if (MovementState == EMovementState::State_Crouch)
        {
            EndCrouch(false);
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

void ASCharacter::StopCrouch()
{
    bHoldingCrouch = false;
    bPerformedSlide = false;
    if (MovementState == EMovementState::State_Slide)
    {
        StopSlide();
    }
}

void ASCharacter::EndCrouch(bool bToSprint)
{
    if (MovementState == EMovementState::State_Crouch || MovementState == EMovementState::State_Slide)
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
void ASCharacter::StartSprint()
{
    bHoldingSprint = true;
    bPerformedSlide = false;
    // Updates the sprint speed
    UpdateMovementValues(EMovementState::State_Sprint);
}

// Stopping to sprint (IE_Released)
void ASCharacter::StopSprint()
{
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

void ASCharacter::StartSlide()
{
    bPerformedSlide = true;
    UpdateMovementValues(EMovementState::State_Slide);
    GetWorldTimerManager().SetTimer(SlideStop, this, &ASCharacter::StopSlide, SlideTime, false, SlideTime);
}

void ASCharacter::StopSlide()
{
    if (MovementState == EMovementState::State_Slide && FloorAngle > -15.0f)
    {
        if (bHoldingSprint)
        {
            EndCrouch(true);
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
        GetWorldTimerManager().SetTimer(SlideStop, this, &ASCharacter::StopSlide, 0.1f, false, 0.1f);
    }
}

void ASCharacter::CheckVault()
{
    float ForwardVelocity = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
    // When you have a if statement, that end at the end of the function, just invert the statement and call return. Is way faster.
    if (!(ForwardVelocity > 0 && !bIsVaulting && GetCharacterMovement()->IsFalling())) return;

    // store those for future use.
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

    FVector forwardAddition = UKismetMathLibrary::GetForwardVector(ColliderRotation) * 5;
    float calucationHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2;
    float scaledCapsuleWthoutHemi = GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();
    for (i = 0; i <= VaultTraceAmount; i++)
    {
        SecondaryVaultStartLocation += forwardAddition;
        SecondaryVaultEndLocation += forwardAddition;
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
        CalculationVector.Z = calucationHeight;
        DownTracePoint += CalculationVector;
        StartLocation = DownTracePoint;
        StartLocation.Z += scaledCapsuleWthoutHemi;
        EndLocation = DownTracePoint;
        EndLocation.Z -= scaledCapsuleWthoutHemi;

        //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
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

    FVector calculationVector = FVector::ZeroVector;
    calculationVector.Z = (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2);
    DownTracePoint += calculationVector;
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

void ASCharacter::CheckAngle()
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
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("%f"),FloorAngle), true);
        }
    }
}

void ASCharacter::Vault(const FTransform TargetTransform)
{
    VaultStartLocation = GetActorTransform();
    VaultEndLocation = TargetTransform;
    UpdateMovementValues(EMovementState::State_Vault);
    HandsMeshComp->GetAnimInstance()->Montage_Play(VaultMontage, 1.0f);
    VaultTimeline.PlayFromStart();
}

// Function that determines the player's maximum speed and other related variables based on movement state
void ASCharacter::UpdateMovementValues(EMovementState NewMovementState)
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


void ASCharacter::UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, bool bSpawnPickup, FWeaponDataStruct CurrentWeaponDataStruct, FWeaponDataStruct NewWeaponDataStruct,
    bool bStatic, FTransform PickupTransform, bool bIsPrimaryWeapon)
{
    
    if (bIsPrimary == bIsPrimaryWeapon/* || (bIsPrimaryWeapon && !PrimaryWeapon) || (!bIsPrimaryWeapon && !SecondaryWeapon)*/)
    {
        return;
    }
    
    if (CurrentWeapon)
    {
        if (IsValid(CurrentWeapon->WeaponData->WeaponUnequip))
        {
            // Calculating the animtime and playing the animation
            const float AnimTime = HandsMeshComp->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponData->WeaponUnequip);

            // Constructing the timer delegate
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(this, FName("SpawnNewWeapon"), NewWeapon, bSpawnPickup, CurrentWeaponDataStruct, NewWeaponDataStruct, bStatic, PickupTransform, bIsPrimaryWeapon);

            // Setting the timer
            GetWorldTimerManager().SetTimer(WeaponEquip, TimerDelegate, AnimTime, false);
        }
        else
        {
            SpawnNewWeapon(NewWeapon, bSpawnPickup, CurrentWeaponDataStruct, NewWeaponDataStruct, bStatic, PickupTransform, bIsPrimaryWeapon);
        }
    }
    else
    {
        SpawnNewWeapon(NewWeapon, bSpawnPickup, CurrentWeaponDataStruct, NewWeaponDataStruct, bStatic, PickupTransform, bIsPrimaryWeapon);
    }
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void ASCharacter::SpawnNewWeapon(TSubclassOf<ASWeaponBase> NewWeapon, bool bSpawnPickup,
                               FWeaponDataStruct CurrentWeaponDataStruct, FWeaponDataStruct NewWeaponDataStruct, bool bStatic, FTransform PickupTransform, bool bIsPrimaryWeapon)
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
            NewPickup->DataStruct = CurrentWeaponDataStruct;
            NewPickup->AttachmentArray = CurrentWeaponDataStruct.WeaponAttachments;
            NewPickup->SpawnAttachmentMesh();
        }
    }


    if (CurrentWeapon)
    {
        CurrentWeapon->K2_DestroyActor();
    }

    // Spawns the new weapon and sets the player as it's owner
    CurrentWeapon = GetWorld()->SpawnActor<ASWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (CurrentWeapon)
    {
        CurrentWeapon->SetOwner(this);
        CurrentWeapon->AttachToComponent(HandsMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeapon->WeaponData->WeaponAttachmentSocketName);

        CurrentWeapon->SpawnAttachments(NewWeaponDataStruct.WeaponAttachments);
        

        if (IsValid(CurrentWeapon->WeaponData->WeaponEquip))
        {        
            HandsMeshComp->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponData->WeaponEquip, 1.0f);
        }
    }

    this->bIsPrimary = bIsPrimaryWeapon;
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

FText ASCharacter::GetCurrentWeaponLoadedAmmo() const
{
    return bIsPrimary? FText::AsNumber(PrimaryWeaponCacheMap.ClipSize) : FText::AsNumber(SecondaryWeaponCacheMap.ClipSize);
}

FText ASCharacter::GetCurrentWeaponRemainingAmmo() const
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

// Called every frame
void ASCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Crouching
	// Sets the new Target Half Height based on whether the player is crouching or standing
	const float TargetHalfHeight = (MovementState == EMovementState::State_Crouch || MovementState == EMovementState::State_Slide)? FinalCapsuleHalfHeight : DefaultCapsuleHalfHeight;
	// Interpolates between the current height and the target height
	const float NewHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetHalfHeight, DeltaTime, CrouchSpeed);
	// Sets the half height of the capsule component to the new interpolated half height
	GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);

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

    // Timeline tick
    VaultTimeline.TickTimeline(DeltaTime);

    CheckAngle();


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
    //DECLARE_DELEGATE_SevenParams(FWeaponSwapDelegate, TSubclassOf<ASWeaponBase>&, bool, FWeaponDataStruct&, FWeaponDataStruct&, bool, FTransform, bool);

    //PlayerInputComponent->BindAction<FWeaponSwapDelegate>("PrimaryWeapon", IE_Pressed, this, &ASCharacter::UpdateWeapon, PrimaryWeapon, false, SecondaryWeaponCacheMap, PrimaryWeaponCacheMap,
    //false, FTransform::Identity, true);

    //PlayerInputComponent->BindAction<FWeaponSwapDelegate>("SecondaryWeapon", IE_Pressed, this, &ASCharacter::UpdateWeapon, SecondaryWeapon, false, PrimaryWeaponCacheMap, SecondaryWeaponCacheMap,
    //false, FTransform::Identity, false);

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

    PlayerInputComponent->BindAction("ScrollUp", IE_Pressed, this, &ASCharacter::ScrollWeapon);
}
