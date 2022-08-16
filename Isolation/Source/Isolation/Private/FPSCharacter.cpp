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
#include "InteractInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WeaponBase.h"
#include "WeaponPickup.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/InputSettings.h"
#include "Isolation/Isolation.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widgets/HUDWidget.h"

void AFPSCharacter::CreateSettingsMenu()
{
    if (PlayerPauseWidget)
    {
        PlayerPauseWidget->RemoveFromParent();
        if (PlayerSettingsWidget)
        {
            PlayerSettingsWidget->AddToViewport();
            CurrentWidget = PlayerSettingsWidget;
        }
    }
}

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

    // Initialising our widgets, and adding the HUD widget to the screen
    if (IsValid(HUDWidget))
    {
        PlayerHudWidget = Cast<USHUDWidget>(CreateWidget(GetWorld(), HUDWidget));
        if (PlayerHudWidget != nullptr)
        {
            PlayerHudWidget->AddToViewport();
        }
    }
    if (IsValid(PauseWidget))
    {
        PlayerPauseWidget = Cast<UPauseWidget>(CreateWidget(GetWorld(), PauseWidget));
    }
    if (IsValid(SettingsWidget))
    {
        PlayerSettingsWidget = Cast<USettingsWidget>(CreateWidget(GetWorld(), SettingsWidget));
    }
    
    GetCharacterMovement()->MaxWalkSpeed = MovementDataMap[EMovementState::State_Walk].MaxWalkSpeed;
    DefaultSpringArmOffset = SpringArmComp->GetRelativeLocation().Z; // Setting the default location of the spring arm

    // Binding a timeline to our vaulting curve
    if (VaultTimelineCurve)
    {
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
        VaultTimeline.AddInterpFloat(VaultTimelineCurve, TimelineProgress);
    }

    EquippedWeapons.Reserve(NumberOfWeaponSlots);
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

void AFPSCharacter::WorldInteract()
{
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;
    TraceParams.AddIgnoredActor(this);
    const FVector CameraLocation = CameraComp->GetComponentLocation();
    const FRotator CameraRotation = CameraComp->GetComponentRotation();
    const FVector TraceDirection = CameraRotation.Vector();
    const FVector TraceEndLocation = CameraLocation + TraceDirection * InteractDistance;

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
    const FVector TraceEndLocation = CameraLocation + TraceDirection * InteractDistance;

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

void AFPSCharacter::ManageOnScreenWidgets()
{
    ASCharacterController* CharacterController = Cast<ASCharacterController>(GetController());
    
    if (CurrentWidget != nullptr)
    {
        if (CurrentWidget == PlayerPauseWidget)
        {
            PlayerPauseWidget->RemoveFromParent();
            PlayerHudWidget->AddToViewport();
            CharacterController->SetInputMode(FInputModeGameOnly());
            CharacterController->SetShowMouseCursor(false);
            UGameplayStatics::SetGamePaused(GetWorld(), false);
            CurrentWidget = nullptr;
        }
        else
        {
            PlayerSettingsWidget->RemoveFromParent();
            PlayerPauseWidget->AddToViewport();
            CurrentWidget = PlayerPauseWidget;
        }
    }
    else
    {
        PlayerHudWidget->RemoveFromParent();
        PlayerPauseWidget->AddToViewport();
        CurrentWidget = PlayerPauseWidget;
        CharacterController->SetInputMode(FInputModeGameAndUI());
        CharacterController->SetShowMouseCursor(true);
        UGameplayStatics::SetGamePaused(GetWorld(), true);
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

// Swapping weapons with the scroll wheel
void AFPSCharacter::ScrollWeapon(const FInputActionValue& Value)
{
    int NewID;
    
    if (Value[0] < 0)
    {        
        NewID = FMath::Clamp(CurrentWeaponSlot + 1, 0, NumberOfWeaponSlots - 1);

        if (CurrentWeaponSlot == NumberOfWeaponSlots - 1)
        {
            NewID = 0;
        }
    }
    else
    {        
        NewID = FMath::Clamp(CurrentWeaponSlot - 1, 0, NumberOfWeaponSlots - 1);

        if (CurrentWeaponSlot == 0)
        {
            NewID = NumberOfWeaponSlots - 1;
        }
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Wheee"));
    SwapWeapon(NewID);
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

    if (Value.GetMagnitude() != 0.0f && CurrentWeapon)
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
    if (CurrentWeapon)
    {
        CurrentWeapon->bCanFire = MovementDataMap[MovementState].bCanFire;
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

void AFPSCharacter::SwapWeapon(int SlotId)
{
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, FString::FromInt(SlotId));

    if (CurrentWeaponSlot == SlotId) { return; }
    if (!EquippedWeapons.Contains(SlotId)) { return; }
    
    CurrentWeaponSlot = SlotId;
    
    if (CurrentWeapon)
    {
        CurrentWeapon->PrimaryActorTick.bCanEverTick = false;
        CurrentWeapon->SetActorHiddenInGame(true);
        CurrentWeapon->StopFire();
    }
    
    CurrentWeapon = EquippedWeapons[SlotId];
    if (CurrentWeapon)
    {
        CurrentWeapon->PrimaryActorTick.bCanEverTick = true;
        CurrentWeapon->SetActorHiddenInGame(false);
        if (CurrentWeapon->WeaponData.WeaponEquip)
        {
            HandsMeshComp->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponEquip, 1.0f);
        }
    }
}

// Spawns a new weapon (either from weapon swap or picking up a new weapon)
void AFPSCharacter::UpdateWeapon(TSubclassOf<ASWeaponBase> NewWeapon, int InventoryPosition, bool bSpawnPickup,
                      bool bStatic, FTransform PickupTransform, FRuntimeWeaponData DataStruct)
{
    // Determining spawn parameters (forcing the weapon to spawn at all times)
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (InventoryPosition == CurrentWeaponSlot && EquippedWeapons.Contains(InventoryPosition))
    {
        if (bSpawnPickup)
        {
            // Calculating the location where to spawn the new weapon in front of the player
            const FVector TraceStart = CameraComp->GetComponentLocation();
            const FRotator TraceStartRotation = CameraComp->GetComponentRotation();
            const FVector TraceDirection = TraceStartRotation.Vector();
            const FVector TraceEnd = TraceStart + TraceDirection * WeaponSpawnDistance;

            // Spawning the new pickup
            ASWeaponPickup* NewPickup = GetWorld()->SpawnActor<ASWeaponPickup>(CurrentWeapon->WeaponData.PickupReference, TraceEnd, FRotator::ZeroRotator, SpawnParameters);
            if (bStatic)
            {
                NewPickup->MainMesh->SetSimulatePhysics(false);
                NewPickup->SetActorTransform(PickupTransform);
            }
            // Applying the current weapon data to the pickup
            NewPickup->bStatic = bStatic;
            NewPickup->bRuntimeSpawned = true;
            NewPickup->WeaponReference = EquippedWeapons[InventoryPosition]->GetClass();
            NewPickup->DataStruct = EquippedWeapons[InventoryPosition]->GeneralWeaponData;
            NewPickup->SpawnAttachmentMesh();
            EquippedWeapons[InventoryPosition]->Destroy();
        }
    }
    // Spawns the new weapon and sets the player as it's owner
    ASWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<ASWeaponBase>(NewWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (SpawnedWeapon)
    {
        SpawnedWeapon->SetOwner(this);
        SpawnedWeapon->AttachToComponent(HandsMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnedWeapon->WeaponData.WeaponAttachmentSocketName);
        SpawnedWeapon->GeneralWeaponData = DataStruct;
        SpawnedWeapon->SpawnAttachments();
        EquippedWeapons.Add(InventoryPosition, SpawnedWeapon);

        if (CurrentWeapon)
        {
            CurrentWeapon->PrimaryActorTick.bCanEverTick = false;
            CurrentWeapon->SetActorHiddenInGame(true);
        }
    
        CurrentWeapon = EquippedWeapons[InventoryPosition];
        CurrentWeaponSlot = InventoryPosition; 
        
        if (CurrentWeapon)
        {
            CurrentWeapon->PrimaryActorTick.bCanEverTick = true;
            CurrentWeapon->SetActorHiddenInGame(false);
            if (CurrentWeapon->WeaponData.WeaponEquip)
            {
                HandsMeshComp->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponEquip, 1.0f);
            }
        }
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
    if (CurrentWeapon)
    {
        if (bIsAiming && CurrentWeapon->WeaponData.bAimingFOV && !CurrentWeapon->bIsReloading)
        {
            TargetFOV = BaseFOV - CurrentWeapon->WeaponData.AimingFOVChange;
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
        for ( int Index = 0; Index < NumberOfWeaponSlots; Index++ )
        {
            if (EquippedWeapons.Contains(Index))
            {
                GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(EquippedWeapons[Index]->GeneralWeaponData.ClipSize));
                GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(EquippedWeapons[Index]->GeneralWeaponData.ClipCapacity));
                GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(EquippedWeapons[Index]->GeneralWeaponData.WeaponHealth));
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

    // Checks to see if we are facing something to interact with, and updates the interaction indicator accordingly
    InteractionIndicator();

    //Updating the scope's material parameter collection
    if (CurrentWeapon)
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

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Make sure that we are using a UEnhancedInputComponent; if not, the project is not configured correctly.
    if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {        
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

        if (CrouchAction)
        {
            // Crouching
            PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFPSCharacter::StartCrouch);
            PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AFPSCharacter::ReleaseCrouch);
        }

        if (FiringAction)
        {
            // Firing
            PlayerEnhancedInputComponent->BindAction(FiringAction, ETriggerEvent::Started, this, &AFPSCharacter::StartFire);
            PlayerEnhancedInputComponent->BindAction(FiringAction, ETriggerEvent::Completed, this, &AFPSCharacter::StopFire);
        }
        
        if (PrimaryWeaponAction)
        {
            // Switching to the primary weapon
            PlayerEnhancedInputComponent->BindAction(PrimaryWeaponAction, ETriggerEvent::Started, this, &AFPSCharacter::SwapWeapon<0>);
        }

        if (SecondaryWeaponAction)
        {            
            // Switching to the secondary weapon
            PlayerEnhancedInputComponent->BindAction(SecondaryWeaponAction, ETriggerEvent::Started, this, &AFPSCharacter::SwapWeapon<1>);
        }

        if (ReloadAction)
        {
            // Reloading
            PlayerEnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AFPSCharacter::Reload);
        }

        if (AimAction)
        {
            // Aiming
            PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AFPSCharacter::StartAds);
            PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AFPSCharacter::StopAds);
        }

        if (InteractAction)
        {
            // Interacting with the world
            PlayerEnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFPSCharacter::WorldInteract);
        }

        if (ScrollAction)
        {
            // Scrolling through weapons
            PlayerEnhancedInputComponent->BindAction(ScrollAction, ETriggerEvent::Started, this, &AFPSCharacter::ScrollWeapon);
        }

        if (PauseAction)
        {
            // Pausing the game
            PlayerEnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AFPSCharacter::ManageOnScreenWidgets);
        }
    }
}
