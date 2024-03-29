// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacter.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FPSCharacterController.h"
#include "WeaponBase.h"
#include "AI/AIManager.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/WidgetManagementComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/SpringArmComponent.h"
#include "Isolation/Isolation.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"

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

    // Spawning the document location arrow
    DocInspectLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("DocumentInspectLocationArrow"));
    DocInspectLocation->SetupAttachment(CameraComp);

    // Spawning the bullet flyby component
    FlybyAreaComponent = CreateDefaultSubobject<USphereComponent>(TEXT("FlybyAreaComp"));
    FlybyAreaComponent->SetupAttachment(RootComponent);
    
    DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // setting the default height of the capsule
}

bool AFPSCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation,
    int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible,
    int32* UserData) const
{

    static const FName NAME_AILineOfSight = FName(TEXT("TestPawnLineOfSight"));
    FHitResult HitResult;


    for (const FName BoneName : DetectionSocketBoneNames)
    {
        const FVector SocketLocation = HandsMeshComp->GetSocketLocation(BoneName);
        
        const bool bHitSocket = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, SocketLocation, FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic)), FCollisionQueryParams(NAME_AILineOfSight, true, IgnoreActor));
        NumberOfLoSChecksPerformed++;

        if (bHitSocket == false || (HitResult.Actor.IsValid() && HitResult.Actor->IsOwnedBy(this)))
        {
            OutSeenLocation = SocketLocation;
            OutSightStrength = 1;
            return true;
        }
    }

    const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, GetActorLocation(), FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic)), FCollisionQueryParams(NAME_AILineOfSight, true, IgnoreActor));
    NumberOfLoSChecksPerformed++;
    if (bHit == false || (HitResult.Actor.IsValid() && HitResult.Actor->IsOwnedBy(this)))
    {
        OutSeenLocation = GetActorLocation();
        OutSightStrength = 1;
        return true;
    }

    OutSightStrength = 0;
    return false;
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

    // Obtaining our inventory component and reserving space in memory for our set of weapons
    if (UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>())
    {
        InventoryComponent = InventoryComp;
        InventoryComponent->GetEquippedWeapons().Reserve(InventoryComponent->GetNumberOfWeaponSlots());
    }

    UAIManager* AIManagerSubsystem = GetWorld()->GetSubsystem<UAIManager>();
    if (AIManagerSubsystem)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Grabbed AI Subsystem"));
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, AIManagerSubsystem->GetName());
    }
}

void AFPSCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    // Make sure that we have a valid PlayerController.
    if (const AFPSCharacterController* PlayerController = Cast<AFPSCharacterController>(GetController()))
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

void AFPSCharacter::UpdateInputMappingContext(const UInputMappingContext* NewMappingContext)
{
    // Make sure that we have a valid PlayerController.
    if (const AFPSCharacterController* PlayerController = Cast<AFPSCharacterController>(GetController()))
    {
        // Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            // PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
            Subsystem->ClearAllMappings();

            // Add each mapping context, along with their priority values. Higher values outprioritize lower values.
            Subsystem->AddMappingContext(NewMappingContext, BaseMappingPriority);
        }
    } 
}

void AFPSCharacter::ResetInputMappingContext()
{
    // Make sure that we have a valid PlayerController.
    if (const AFPSCharacterController* PlayerController = Cast<AFPSCharacterController>(GetController()))
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

// Playing footstep sounds on FootstepAudioComp, called from animnotifies
void AFPSCharacter::FootstepSounds()
{
    // Calculating the line trace start and end locations
    const FVector TraceStart = GetActorLocation();
    FVector TraceEnd = TraceStart;
    TraceEnd.Z -= 100.0f;

    // Query parameters for the interaction line trace
    FCollisionQueryParams QueryParams;
    QueryParams.bReturnPhysicalMaterial = true;

    FootstepAudioComp->SetIntParameter(FName("floor"), 0);
    if(GetWorld()->LineTraceSingleByChannel(FootstepHit, TraceStart, TraceEnd, FOOTSTEP_TRACE, QueryParams))
    {
        // Updating the relevant parameter in the footstep audio component if we hit something with our line trace
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
    // Storing movement vectors for animation manipulation
    ForwardMovement = Value[1];
    RightMovement = Value[0];

    // Moving the player
    if (Value.GetMagnitude() != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value[1]);
        AddMovementInput(GetActorRightVector(), Value[0]);
    }
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
    // Storing look vectors for animation manipulation
    MouseX = Value[1];
    MouseY = Value[0];

    // Looking around
    AddControllerPitchInput(Value[1] * -1);
    AddControllerYawInput(Value[0]);
    
    if (InventoryComponent)
    {
        if (Value.GetMagnitude() != 0.0f && InventoryComponent->GetCurrentWeapon())
        {
            // If movement is detected and we have a current weapon, make sure we don't recover the recoil
            InventoryComponent->GetCurrentWeapon()->SetShouldRecover(false);
            InventoryComponent->GetCurrentWeapon()->GetRecoilRecoveryTimeline()->Stop();
        }
    }
}

void AFPSCharacter::StartCrouch()
{
    bHoldingCrouch = true;
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        if (MovementState == EMovementState::State_Crouch)
        {
            StopCrouch(false);
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
        // If we are in the air and have not performed a slide yet
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

void AFPSCharacter::StartSprint()
{
    if (!HasSpaceToStandUp() && (MovementState == EMovementState::State_Crouch || MovementState ==
        EMovementState::State_Slide))
    {
        return;
    }
    bHoldingSprint = true;
    bPerformedSlide = false;
    UpdateMovementValues(EMovementState::State_Sprint);
}

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

    // Store these for future use.
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

    // Checking if we are near a wall
    if (!GetWorld()->SweepSingleByChannel(MantleHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic,
                                          FCollisionShape::MakeCapsule(30, 50), TraceParams)) return;
    if (!MantleHit.bBlockingHit) return;
    
    FVector ForwardImpactPoint = MantleHit.ImpactPoint;
    FVector ForwardImpactNormal = MantleHit.ImpactNormal;
    FVector CapsuleLocation = ForwardImpactPoint;
    CapsuleLocation.Z = ColliderLocation.Z;
    CapsuleLocation += ForwardImpactNormal * -15;
    StartLocation = CapsuleLocation;
    StartLocation.Z += 100;
    EndLocation = CapsuleLocation;

    // Checking if we can stand up on the wall that we've hit
    if (!GetWorld()->SweepSingleByChannel(MantleHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(1), TraceParams)) return;
    if (!GetCharacterMovement()->IsWalkable(MantleHit)) return;
    
    FVector SecondaryVaultStartLocation = MantleHit.ImpactPoint;
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

    // Tracing downwards VaultTraceAmount times and looking for a significant change in height followed by a space large enough to stand
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

        // If we find such a location, break the loop and vault
        ForwardImpactNormal.X -= 1;
        ForwardImpactNormal.Y -= 1;
        VaultTargetLocation = FTransform(UKismetMathLibrary::MakeRotFromX(ForwardImpactNormal), DownTracePoint);
        bIsVaulting = true;
        Vault(VaultTargetLocation);
        bVaultFailed = false;
        break;
    }

    if (!bVaultFailed) return;

    // If the vault has failed (there is no space or the surface is too high), we proceed to perform the mantle logic
    
    FVector DownTracePoint = MantleHit.Location;
    DownTracePoint.Z = MantleHit.ImpactPoint.Z;
 
    FVector CalculationVector = FVector::ZeroVector;
    CalculationVector.Z = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2;
    DownTracePoint += CalculationVector;
    StartLocation = DownTracePoint;
    StartLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();
    EndLocation = DownTracePoint;
    EndLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere();

    // Looking for a safe place to mantle to
     if (GetWorld()->SweepSingleByChannel(MantleHit, StartLocation, EndLocation, FQuat::Identity, ECC_WorldStatic,
                                         FCollisionShape::MakeSphere(GetCapsuleComponent()->GetUnscaledCapsuleRadius()),
                                         TraceParams)) return;

    ForwardImpactNormal.X -= 1;
    ForwardImpactNormal.Y -= 1;
    VaultTargetLocation = FTransform(UKismetMathLibrary::MakeRotFromX(ForwardImpactNormal), DownTracePoint);
    bIsVaulting = true;

    // Calling vault with our mantle target point
    Vault(VaultTargetLocation);
}

// Progresses the timeline that is used to vault the character
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

void AFPSCharacter::CheckAngle(float DeltaTime)
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);

    // Determines the angle of the floor from the vector of a hit line trace
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

    // Check to see if a capsule collision collides with the environment, if yes, we don't have space to stand up
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
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Stand up trace returned hit", true);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, StandUpHit.Actor->GetName());
        }
        return false;
    }

    return true;
}

void AFPSCharacter::Vault(const FTransform TargetTransform)
{
    // Updating our target location and playing the vault timeline from start
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
            InventoryComponent->GetCurrentWeapon()->SetCanFire(MovementDataMap[MovementState].bCanFire);
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

    // Vignette
    if (VignetteMappingCurve != nullptr)
    {
        const float NewVignetteIntensity = FMath::FInterpTo(CameraComp->PostProcessSettings.VignetteIntensity, VignetteMappingCurve->GetFloatValue(BreathHealth), DeltaTime, CameraVignetteInterpSpeed);
        CameraComp->PostProcessSettings.VignetteIntensity = NewVignetteIntensity;
    }
    
    // FOV adjustments
    float TargetFOV = ((MovementState == EMovementState::State_Sprint || MovementState == EMovementState::State_Slide) && GetVelocity().Size() > MovementDataMap[EMovementState::State_Walk].MaxWalkSpeed)? BaseFOV + SpeedFOVChange : BaseFOV;
    if (InventoryComponent)
    {
        if (InventoryComponent->GetCurrentWeapon())
        {
            if (bIsAiming && InventoryComponent->GetCurrentWeapon()->GetStaticWeaponData()->bAimingFOV && !InventoryComponent->GetCurrentWeapon()->IsReloading())
            {
                TargetFOV = BaseFOV - InventoryComponent->GetCurrentWeapon()->GetStaticWeaponData()->AimingFOVChange;
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
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(InventoryComponent->GetEquippedWeapons()[Index]->GetRuntimeWeaponData()->ClipSize));
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(InventoryComponent->GetEquippedWeapons()[Index]->GetRuntimeWeaponData()->ClipCapacity));
                    GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(InventoryComponent->GetEquippedWeapons()[Index]->GetRuntimeWeaponData()->WeaponHealth));
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

void AFPSCharacter::PlayFlybySounds(FHitResult Hit)
{
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), FlybySounds, Hit.Location);
    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, "Playing flyby sounds");
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
            InteractionComponent->InteractAction = InteractAction;
            InteractionComponent->SetupInputComponent(PlayerEnhancedInputComponent);
        }

        if (UWidgetManagementComponent* WidgetManagementComponent = FindComponentByClass<UWidgetManagementComponent>())
        {
            WidgetManagementComponent->PauseAction = PauseAction;
            WidgetManagementComponent->SetupInputComponent(PlayerEnhancedInputComponent);
        }

        if (UInventoryComponent* InventoryComp = FindComponentByClass<UInventoryComponent>())
        {            
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
