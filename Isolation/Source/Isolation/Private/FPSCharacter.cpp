// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacter.h"
#include "AmmoPickup.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "FPSCharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WeaponBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/InputSettings.h"
#include "Isolation/Isolation.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/WidgetManagmentComponent.h"

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
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
    Super::BeginPlay();
        
    GetCharacterMovement()->MaxWalkSpeed = MovementDataMap[EMovementState::State_Walk].MaxWalkSpeed;
    DefaultSpringArmOffset = SpringArmComp->GetRelativeLocation().Z; // Setting the default location of the spring arm

    // Binding a timeline to our vaulting curve
    if (VaultTimelineCurve)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
        VaultTimeline.AddInterpFloat(VaultTimelineCurve, TimelineProgress);
    }

    if (UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>())
    {
        InventoryComponent = InventoryComp;
        InventoryComponent->GetEquippedWeapons().Reserve(InventoryComponent->GetNumberOfWeaponSlots());
    }
}

void AFPSCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    // Make sure that we have a valid PlayerController.
    if (const ASCharacterController* PlayerController = Cast<ASCharacterController>(GetController()))
    {
        // Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            // PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
            Subsystem->ClearAllMappings();

            // Add each mapping context, along with their priority values. Higher values outprioritize lower values.
            Subsystem->AddMappingContext(BaseMappingContext, BaseMappingPriority);
        }
    }
}

/** Progresses the timeline that is used to vault the character */
void AFPSCharacter::TimelineProgress(const float Value)
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

// Playing footstep sounds on FootstepAudioComp, called from animnotifies
void AFPSCharacter::FootstepSounds()
{
    const FVector TraceStart = GetActorLocation();
    FVector TraceEnd = TraceStart;
    TraceEnd.Z -= 100.0f;

    // Query parameters for the interaction line trace
    FCollisionQueryParams QueryParams;
    QueryParams.bReturnPhysicalMaterial = true;
    
    FootstepAudioComp->SetIntParameter(FName("floor"), 0);
    if(GetWorld()->LineTraceSingleByChannel(FootstepHit, TraceStart, TraceEnd, FOOTSTEP_TRACE, QueryParams))
    {
        FootstepAudioComp->SetIntParameter(FName("floor"), SurfaceMaterialArray.Find(FootstepHit.PhysMaterial.Get()));
        if (bDrawDebug)
        {
            DrawDebugLine(GetWorld(), TraceStart, FootstepHit.Location, FColor::Red, false, 10.0f, 0.0f, 2.0f);
        }
    }
    else if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 10.0f, 0.0f, 2.0f);
    }

    FootstepAudioComp->Play();
}

void AFPSCharacter::Move(const FInputActionValue& Value)
{
    ForwardMovement = Value[1];
    RightMovement = Value[0];
    
    if (Value.GetMagnitude() != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value[1]);
        AddMovementInput(GetActorRightVector(), Value[0]);
    }
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
    MouseX = Value[1];
    MouseY = Value[0];
    
    AddControllerPitchInput(Value[1] * -1);
    AddControllerYawInput(Value[0]);

    if (InventoryComponent)
    {
        if (Value.GetMagnitude() != 0.0f && InventoryComponent->GetCurrentWeapon())
        {
            InventoryComponent->GetCurrentWeapon()->bShouldRecover = false;
            InventoryComponent->GetCurrentWeapon()->RecoilRecoveryTimeline.Stop();
        }
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
    if (!HasSpaceToStandUp() && (MovementState == EMovementState::State_Crouch || MovementState ==
        EMovementState::State_Slide))
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
    if (MovementState == EMovementState::State_Slide && bHoldingCrouch)
    {
        UpdateMovementValues(EMovementState::State_Crouch);
    }
    else if (MovementState == EMovementState::State_Sprint)
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

void AFPSCharacter::StartAds()
{
    bWantsToAim = true;
}

void AFPSCharacter::StopAds()
{
    bWantsToAim = false;
}


void AFPSCharacter::CheckVault()
{    
    float ForwardVelocity = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
    if (!(ForwardVelocity > 0 && !bIsVaulting && GetCharacterMovement()->IsFalling())) return;

    // store these for future use.
    FVector ColliderLocation = GetCapsuleComponent()->GetComponentLocation();
    FRotator ColliderRotation = GetCapsuleComponent()->GetComponentRotation();


    FVector StartLocation = ColliderLocation;
    FVector EndLocation = ColliderLocation + UKismetMathLibrary::GetForwardVector(ColliderRotation) * 75;
    if (bDrawDebug)
    {
        DrawDebugCapsule(GetWorld(), StartLocation, 50, 30, FQuat::Identity, FColor::Red);
    }

    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);

    if (!GetWorld()->SweepSingleByChannel(VaultHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeCapsule(30, 50), TraceParams)) return;
    if (!VaultHit.bBlockingHit) return;
    
    FVector ForwardImpactPoint = VaultHit.ImpactPoint;
    FVector ForwardImpactNormal = VaultHit.ImpactNormal;
    FVector CapsuleLocation = ForwardImpactPoint;
    CapsuleLocation.Z = ColliderLocation.Z;
    CapsuleLocation += ForwardImpactNormal * -15;

    StartLocation = CapsuleLocation;
    StartLocation.Z += 100;
    EndLocation = CapsuleLocation;

    if (!GetWorld()->SweepSingleByChannel(VaultHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(1), TraceParams)) return;
    if (!GetCharacterMovement()->IsWalkable(VaultHit)) return;
    
    FVector SecondaryVaultStartLocation = VaultHit.ImpactPoint;
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

        float TraceLength = SecondaryVaultStartLocation.Z - VaultHit.ImpactPoint.Z;
        if (!bInitialSwitch)
        {
            InitialTraceHeight = TraceLength;
            bInitialSwitch = true;
        }

        PreviousTraceHeight = CurrentTraceHeight;
        CurrentTraceHeight = TraceLength;
        if (!(!(FMath::IsNearlyEqual(CurrentTraceHeight, InitialTraceHeight, 20.0f)) && CurrentTraceHeight < MaxMantleHeight)) continue;

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
        VaultTargetLocation = FTransform(UKismetMathLibrary::MakeRotFromX(ForwardImpactNormal), DownTracePoint);
        bIsVaulting = true;
        Vault(VaultTargetLocation);
        bVaultFailed = false;
        break;
    }

    if (!bVaultFailed) return;
    
    FVector DownTracePoint = VaultHit.Location;
    DownTracePoint.Z = VaultHit.ImpactPoint.Z;
    //UPrimitiveComponent* hitComponent = hit.Component;

    FVector CalculationVector = FVector::ZeroVector;
    CalculationVector.Z = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2;
    DownTracePoint += CalculationVector;
    StartLocation = DownTracePoint;
    StartLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();
    EndLocation = DownTracePoint;
    EndLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();

    //DrawDebugSphere(GetWorld(), startLocation, GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 32, FColor::Green);
    if (GetWorld()->SweepSingleByChannel(VaultHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic,
                                         FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()),
                                         TraceParams)) return;

    ForwardImpactNormal.X -= 1;
    ForwardImpactNormal.Y -= 1;
    VaultTargetLocation = FTransform(UKismetMathLibrary::MakeRotFromX(ForwardImpactNormal), DownTracePoint);
    bIsVaulting = true;
    
    Vault(VaultTargetLocation);
}

void AFPSCharacter::CheckAngle(float DeltaTime)
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);

    FVector CapsuleHeight = GetCapsuleComponent()->GetComponentLocation();
    CapsuleHeight.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const FVector AngleStartTrace = CapsuleHeight;
    FVector AngleEndTrace = AngleStartTrace;
    AngleEndTrace.Z -= 50;
    if (GetWorld()->LineTraceSingleByChannel(AngleHit, AngleStartTrace, AngleEndTrace, ECC_WorldStatic, TraceParams))
    {
        const FVector FloorVector = AngleHit.ImpactNormal;
        const FRotator FinalRotation = UKismetMathLibrary::MakeRotFromZX(FloorVector, GetActorForwardVector());
        FloorAngle = FinalRotation.Pitch;
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red,
                                             FString::Printf(TEXT("Current floor angle = %f"), FloorAngle), true);
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
            
    if (GetWorld()->SweepSingleByChannel(StandUpHit, CenterVector, CenterVector, FQuat::Identity, STAND_UP_CHECK_COLLISION, CollisionCapsule))
    {
        /* confetti or smth idk */
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "SweepSingleByChannel returned true", true);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, StandUpHit.Actor->GetName());
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
    // Clearing sprinting and crouching flags
    bIsSprinting = false;
    bIsCrouching = false;

    // Updating the movement state
    MovementState = NewMovementState;

    // Updating CharacterMovementComponent variables based on movement state
    if (InventoryComponent)
    {
        if (InventoryComponent->GetCurrentWeapon())
        {
            InventoryComponent->GetCurrentWeapon()->bCanFire = MovementDataMap[MovementState].bCanFire;
        }
    }
    GetCharacterMovement()->MaxAcceleration = MovementDataMap[MovementState].MaxAcceleration;
    GetCharacterMovement()->BrakingDecelerationWalking = MovementDataMap[MovementState].BreakingDecelerationWalking;
    GetCharacterMovement()->GroundFriction = MovementDataMap[MovementState].GroundFriction;
    GetCharacterMovement()->MaxWalkSpeed = MovementDataMap[MovementState].MaxWalkSpeed;

    // Updating sprinting and crouching flags
    if (MovementState == EMovementState::State_Crouch)
    {
        bIsCrouching = true;
    }
    if (MovementState == EMovementState::State_Sprint)
    {
        bIsSprinting = true;
    }

    
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
    const float SpringArmTargetOffset = (MovementState == EMovementState::State_Crouch || MovementState == EMovementState::State_Slide)? DefaultSpringArmOffset + CrouchedSpringArmHeightDelta : DefaultSpringArmOffset;
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
    float TargetFOV = ((MovementState == EMovementState::State_Sprint || MovementState == EMovementState::State_Slide) && GetVelocity().Size() > MovementDataMap[EMovementState::State_Walk].MaxWalkSpeed)? BaseFOV + SpeedFOVChange : BaseFOV;
    if (InventoryComponent)
    {
        if (InventoryComponent->GetCurrentWeapon())
        {
            if (bIsAiming && InventoryComponent->GetCurrentWeapon()->WeaponData.bAimingFOV && !InventoryComponent->GetCurrentWeapon()->bIsReloading)
            {
                TargetFOV = BaseFOV - InventoryComponent->GetCurrentWeapon()->WeaponData.AimingFOVChange;
                FOVChangeSpeed = 6;
            }
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

    // Slide performed check, so that if the player is in the air and presses the slide key, they slide when they land
    if (GetCharacterMovement()->IsMovingOnGround() && !bPerformedSlide && bWantsToSlide)
    {
        StartSlide();
        bWantsToSlide = false;
    }

    // Checks whether we can vault every frame
    CheckVault();

    // Checks the floor angle to determine whether we should keep sliding or not
    CheckAngle(DeltaTime);


    if (bDrawDebug)
    {
        if (InventoryComponent)
        {
            for ( int Index = 0; Index < InventoryComponent->GetNumberOfWeaponSlots(); Index++ )
            {
                if (InventoryComponent->GetEquippedWeapons().Contains(Index))
                {
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(InventoryComponent->GetEquippedWeapons()[Index]->GeneralWeaponData.ClipSize));
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(InventoryComponent->GetEquippedWeapons()[Index]->GeneralWeaponData.ClipCapacity));
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(InventoryComponent->GetEquippedWeapons()[Index]->GeneralWeaponData.WeaponHealth));
                }
                else
                {
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("No Weapon Found"));
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("No Weapon Found"));
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("No Weapon Found"));
                }
                GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::FromInt(Index));
            }
        }
    }

    //Updating the scope's material parameter collection
    if (InventoryComponent)
    {
        if (InventoryComponent->GetCurrentWeapon())
        {
            if (bIsAiming)
            {
                ScopeBlend = FMath::FInterpConstantTo(ScopeBlend, 1, DeltaTime, 8.0f);
                UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScopeOpacityParameterCollection,
                                                                OpacityParameterName, ScopeBlend);
            }
            else
            {
                ScopeBlend = FMath::FInterpConstantTo(ScopeBlend, 0, DeltaTime, 8.0f);
                UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScopeOpacityParameterCollection,
                                                                OpacityParameterName, ScopeBlend);
            }
        }
    }
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

        
    // Make sure that we are using a UEnhancedInputComponent; if not, the project is not configured correctly.
    if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {        
        if (UInteractionComponent* InteractionComponent = FindComponentByClass<UInteractionComponent>())
        {
            InteractionComponent->WorldInteractAction = InteractAction;
            InteractionComponent->SetupInputComponent(PlayerEnhancedInputComponent);
        }

        if (UWidgetManagmentComponent* WidgetManagmentComponent = FindComponentByClass<UWidgetManagmentComponent>())
        {
            WidgetManagmentComponent->PauseAction = PauseAction;
            WidgetManagmentComponent->SetupInputComponent(PlayerEnhancedInputComponent);
        }

        if (UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>())
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("WHOOP"));
            
            InventoryComp->FiringAction = FiringAction;
            InventoryComp->PrimaryWeaponAction = PrimaryWeaponAction;
            InventoryComp->SecondaryWeaponAction = SecondaryWeaponAction;
            InventoryComp->ReloadAction = ReloadAction;
            InventoryComp->ScrollAction = ScrollAction;
            
            InventoryComp->SetupInputComponent(PlayerEnhancedInputComponent);
        }
        
        if (JumpAction)
        {
            // Jumping
            PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFPSCharacter::Jump);
        }
        
        if (SprintAction)
        {
            // Sprinting
            PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AFPSCharacter::StartSprint);
            PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFPSCharacter::StopSprint);
        }

        if (MovementAction)
        {
            // Move forward/back + left/right inputs
            PlayerEnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);
        }

        if (LookAction)
        {
            // Look up/down + left/right
            PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);
        }

        if (AimAction)
        {
            // Aiming
            PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AFPSCharacter::StartAds);
            PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AFPSCharacter::StopAds);
        }

        if (CrouchAction)
        {
            // Crouching
            PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFPSCharacter::StartCrouch);
            PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AFPSCharacter::ReleaseCrouch);
        }
    }
}
