// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponBase.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "SCharacterController.h"
#include "func_lib/SWeaponStatsCalculator.h"
#include "SCharacter.h"
#include "Interfaces/ITargetDevice.h"

// Sets default values
ASWeaponBase::ASWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Creating our weapon's skeletal mesh, telling it to not cast shadows and finally setting it as the root of the actor
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    MeshComp->CastShadow = false;
    RootComponent = MeshComp;

    BarrelAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BarrelAttachment"));
    BarrelAttachment->CastShadow = false;
    BarrelAttachment->SetupAttachment(RootComponent);

    MagazineAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MagazineAttachment"));
    MagazineAttachment->CastShadow = false;
    MagazineAttachment->SetupAttachment(RootComponent);

    SightsAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SightsAttachment"));
    SightsAttachment->CastShadow = false;
    SightsAttachment->SetupAttachment(RootComponent);

    StockAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StockAttachment"));
    StockAttachment->CastShadow = false;
    StockAttachment->SetupAttachment(RootComponent);

    GripAttachment = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GripAttachment"));
    GripAttachment->CastShadow = false;
    GripAttachment->SetupAttachment(RootComponent);
    
    // Default value allowing the weapon to be fired
    bCanFire = true;
    bIsReloading = false;

    // Making our recoil modifiers equal to one, since we are multiplying
    HorizontalRecoilModifier = 1.0f;
    VerticalRecoilModifier = 1.0f;
}


// Called when the game starts or when spawned
void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();
    
    //Sets the default values for our trace query
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

    // Getting a reference to the relevant row in the WeaponData DataTable
    WeaponData = WeaponDataTable->FindRow<FWeaponData>(FName(DataTableNameRef), FString(DataTableNameRef), true);

    /*
     *Setting our default animation values
     *We set these here, but they can be overriden later by variables from applied attachments.
     */
    if (WeaponData)
    {
        if (WeaponData->BS_Walk)
        {
            WalkBlendSpace = WeaponData->BS_Walk;
        }
        if (WeaponData->BS_Ads_Walk)
        {
            ADSWalkBlendSpace = WeaponData->BS_Ads_Walk;
        }
        if (WeaponData->Anim_Idle)
        {
            Anim_Idle = WeaponData->Anim_Idle;
        }
        if (WeaponData->Anim_Sprint)
        {
            Anim_Sprint = WeaponData->Anim_Sprint;
        }
        if (WeaponData->Anim_Ads_Idle)
        {
            Anim_ADS_Idle = WeaponData->Anim_Ads_Idle;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find the weapon's Weapon Data in the DataTable, check to make sure you've set a DataTableNameRef value in your Weapon Blueprint"));
    }


    // Setting our recoil & recovery curves
    if (VerticalRecoilCurve)
    {
        FOnTimelineFloat VerticalRecoilProgressFunction;
        VerticalRecoilProgressFunction.BindUFunction(this, FName("HandleVerticalRecoilProgress"));
        VerticalRecoilTimeline.AddInterpFloat(VerticalRecoilCurve, VerticalRecoilProgressFunction);
    }

    if (HorizontalRecoilCurve)
    {
        FOnTimelineFloat HorizontalRecoilProgressFunction;
        HorizontalRecoilProgressFunction.BindUFunction(this, FName("HandleHorizontalRecoilProgress"));
        HorizontalRecoilTimeline.AddInterpFloat(HorizontalRecoilCurve, HorizontalRecoilProgressFunction);
    }

    if (RecoveryCurve)
    {
        FOnTimelineFloat RecoveryProgressFunction;
        RecoveryProgressFunction.BindUFunction(this, FName("HandleRecoveryProgress"));
        RecoilRecoveryTimeline.AddInterpFloat(RecoveryCurve, RecoveryProgressFunction);
    }
}



void ASWeaponBase::SpawnAttachments(TArray<FName> AttachmentsArray)
{
    if (WeaponData->bHasAttachments)
    {
        for (FName RowName : AttachmentsArray)
        {
            AttachmentData = WeaponData->AttachmentsDataTable->FindRow<FAttachmentData>(RowName, RowName.ToString(), true);

            if (AttachmentData)
            {
                DamageModifier += AttachmentData->BaseDamageImpact;
                WeaponPitchModifier += AttachmentData->WeaponPitchVariationImpact;
                WeaponYawModifier += AttachmentData->WeaponYawVariationImpact;
                HorizontalRecoilModifier += AttachmentData->HorizontalRecoilMultiplier;
                VerticalRecoilModifier += AttachmentData->VerticalRecoilMultiplier;

                if (AttachmentData->AttachmentType == EAttachmentType::Barrel)
                {
                    BarrelAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    WeaponData->MuzzleLocation = AttachmentData->MuzzleLocationOverride;
                    WeaponData->ParticleSpawnLocation = AttachmentData->ParticleSpawnLocationOverride;
                    WeaponData->bSilenced = AttachmentData->bSilenced;
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
                {
                    MagazineAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    WeaponData->FireSound = AttachmentData->FiringSoundOverride;
                    WeaponData->SilencedSound = AttachmentData->SilencedFiringSoundOverride;
                    WeaponData->RateOfFire = AttachmentData->FireRate;
                    WeaponData->bAutomaticFire = AttachmentData->AutomaticFire;
                    WeaponData->VerticalRecoilCurve = AttachmentData->VerticalRecoilCurve;
                    WeaponData->HorizontalRecoilCurve = AttachmentData->HorizontalRecoilCurve;
                    WeaponData->RecoilCameraShake = AttachmentData->RecoilCameraShake;
                    WeaponData->bIsShotgun = AttachmentData->bIsShotgun;
                    WeaponData->ShotgunRange = AttachmentData->ShotgunRange;
                    WeaponData->ShotgunPellets = AttachmentData->ShotgunPellets;
                    WeaponData->EmptyWeaponReload = AttachmentData->EmptyWeaponReload;
                    WeaponData->WeaponReload = AttachmentData->WeaponReload;
                    WeaponData->EmptyPlayerReload = AttachmentData->EmptyPlayerReload;
                    WeaponData->PlayerReload = AttachmentData->PlayerReload;
                    WeaponData->Gun_Shot = AttachmentData->Gun_Shot;
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Sights)
                {
                    SightsAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    VerticalCameraOffset = AttachmentData->VerticalCameraOffset;
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Stock)
                {
                    StockAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                }
                else if (AttachmentData->AttachmentType == EAttachmentType::Grip)
                {
                    GripAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                    if (AttachmentData->BS_Walk)
                    {
                        WalkBlendSpace = AttachmentData->BS_Walk;
                    }
                    if (AttachmentData->BS_Ads_Walk)
                    {
                        ADSWalkBlendSpace = AttachmentData->BS_Ads_Walk;
                    }
                    if (AttachmentData->Anim_Idle)
                    {
                        Anim_Idle = AttachmentData->Anim_Idle;
                    }
                    if (AttachmentData->Anim_Sprint)
                    {
                        Anim_Sprint = AttachmentData->Anim_Sprint;
                    }
                    if (AttachmentData->Anim_Ads_Idle)
                    {
                        Anim_ADS_Idle = AttachmentData->Anim_Ads_Idle;
                    }
                }
            }
        }
    }
}

void ASWeaponBase::StartFire()
{ 
    // If the weapon can be fired (i.e. not reloading, for example)
    if (bCanFire && WeaponData)
    {
        // sets a timer for firing the weapon - if bAutomaticFire is true then this timer will repeat until cleared by StopFire(), leading to fully automatic fire
        GetWorldTimerManager().SetTimer(ShotDelay, this, &ASWeaponBase::Fire, WeaponData->RateOfFire, WeaponData->bAutomaticFire, 0.0f);

        if (bShowDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Started firing timer"));
        }
        
        StartRecoil();
    }
    
}

void ASWeaponBase::StartRecoil()
{
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    const ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

    ShotsFired = 0;
    

    if (bCanFire && (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) > 0 && !bIsReloading && CharacterController)
    {
        VerticalRecoilTimeline.PlayFromStart();
        HorizontalRecoilTimeline.PlayFromStart();
        ControlRotation = CharacterController->GetControlRotation();
        bShouldRecover = true;
    }
}

void ASWeaponBase::EnableFire()
{
    // Fairly self explanatory - allows the weapon to fire again after waiting for an animation to finish or finishing a reload
    bCanFire = true;
}

void ASWeaponBase::StopFire()
{
    
    // Stops the gun firing (for automatic fire)
    GetWorldTimerManager().ClearTimer(ShotDelay);
    VerticalRecoilTimeline.Stop();
    HorizontalRecoilTimeline.Stop();
    RecoilRecovery();
}

void ASWeaponBase::Fire()
{    
    // Casting to the game instance (which stores all the ammunition and health variables)
    ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    // Allowing the gun to fire if it has ammunition, is not reloading and the bCanFire variable is true
    if(bCanFire && (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) > 0 && !bIsReloading)
    {
        // Printing debug strings
        if(bShowDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Fire", true);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::FromInt((PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize)), true);
        }
        

        // Subtracting from the ammunition count of the weapon
        (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) -= 1;

        const int NumberOfShots = WeaponData->bIsShotgun? WeaponData->ShotgunPellets : 1;
        for (int i = 0; i < NumberOfShots; i++)
        {
            // Setting up the parameters we need to do a line trace from the muzzle of the gun and calculating the start and end points of the ray trace
            if (WeaponData->bHasAttachments && BarrelAttachment)
            {
                TraceStart = BarrelAttachment->GetSocketLocation(WeaponData->MuzzleLocation);
                TraceStartRotation = BarrelAttachment->GetSocketRotation(WeaponData->MuzzleLocation);
            }
            else
            {
                TraceStart = MeshComp->GetSocketLocation(WeaponData->MuzzleLocation);
                TraceStartRotation = MeshComp->GetSocketRotation(WeaponData->MuzzleLocation);
            }
            TraceStartRotation.Pitch += FMath::FRandRange(-(WeaponData->WeaponPitchVariation + WeaponPitchModifier), WeaponData->WeaponPitchVariation + WeaponPitchModifier);
            TraceStartRotation.Yaw += FMath::FRandRange(-(WeaponData->WeaponYawVariation + WeaponYawModifier), WeaponData->WeaponYawVariation + WeaponYawModifier);
            TraceDirection = TraceStartRotation.Vector();
            TraceEnd = TraceStart + (TraceDirection * (WeaponData->bIsShotgun? WeaponData->ShotgunRange : WeaponData->LengthMultiplier));

            // Applying Recoil to the weapon
            Recoil();

            // Playing an animation on the weapon mesh
            if (WeaponData->Gun_Shot)
            {
                MeshComp->PlayAnimation(WeaponData->Gun_Shot, false);
            }

            // Drawing a line trace based on the parameters calculated previously 
            if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_GameTraceChannel1, QueryParams))
            {                
                // Drawing debug line trace
                if (bShowDebug)
                {
                    DrawDebugLine(GetWorld(), TraceStart, Hit.Location, FColor::Red, false, 10.0f, 0.0f, 2.0f);
                }
                
                // Resetting finalDamage
                FinalDamage = 0.0f;

                // Setting finalDamage based on the type of surface hit
                FinalDamage = (WeaponData->BaseDamage + DamageModifier);
                
                if (Hit.PhysMaterial.Get() == WeaponData->HeadshotDamageSurface)
                {
                    FinalDamage = (WeaponData->BaseDamage + DamageModifier) * WeaponData->HeadshotMultiplier;
                }

                AActor* HitActor = Hit.GetActor();

                // Applying the previously set damage to the hit actor
                UGameplayStatics::ApplyPointDamage(HitActor, FinalDamage, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
            }

            // Selecting the hit effect based on the hit physical surface material (hit.PhysMaterial.Get()) and spawning it (Niagara)

            if (Hit.PhysMaterial.Get() == WeaponData->NormalDamageSurface || Hit.PhysMaterial.Get() == WeaponData->HeadshotDamageSurface)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponData->EnemyHitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
            }
            else if (Hit.PhysMaterial.Get() == WeaponData->GroundSurface)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponData->GroundHitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
            }
            else if (Hit.PhysMaterial.Get() == WeaponData->RockSurface)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponData->RockHitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
            }
            else
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WeaponData->DefaultHitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
            }
        }

        // Spawning the firing sound
        if(WeaponData->bSilenced)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->SilencedSound, TraceStart);
        }
        else
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->FireSound, TraceStart);
        }

        // Spawning the firing particle effect
        if (ParticleSocketOverride != "")
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponData->MuzzleFlash, BarrelAttachment, ParticleSocketOverride, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true, true);
        }
        else
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponData->MuzzleFlash, MeshComp, WeaponData->ParticleSpawnLocation, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true, true);
        }


        FRotator EjectionSpawnVector = FRotator::ZeroRotator;
        EjectionSpawnVector.Yaw = 270.0f;
        UNiagaraFunctionLibrary::SpawnSystemAttached(EjectedCasing, MagazineAttachment, FName("ejection_port"), FVector::ZeroVector, EjectionSpawnVector, EAttachLocation::SnapToTarget, true, true);

        if (!WeaponData->bAutomaticFire)
        {
            VerticalRecoilTimeline.Stop();
            HorizontalRecoilTimeline.Stop();
            RecoilRecovery();
        }
    }
    else if (bCanFire && !bIsReloading)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->EmptyFireSound, MeshComp->GetSocketLocation(WeaponData->MuzzleLocation));
        // Clearing the ShotDelay timer so that we don't have a constant ticking when the player has no ammo, just a single click
        GetWorldTimerManager().ClearTimer(ShotDelay);

        RecoilRecovery();
    }
    
}

void ASWeaponBase::Recoil()
{
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

    // Apply recoil
    if (WeaponData->bAutomaticFire && CharacterController && ShotsFired > 0)
    {
        CharacterController->AddPitchInput(WeaponData->VerticalRecoilCurve->GetFloatValue(VerticalRecoilTimeline.GetPlaybackPosition()) * VerticalRecoilModifier);
        CharacterController->AddYawInput(WeaponData->HorizontalRecoilCurve->GetFloatValue(HorizontalRecoilTimeline.GetPlaybackPosition()) * HorizontalRecoilModifier);
    }
    else
    {
        CharacterController->AddPitchInput(WeaponData->VerticalRecoilCurve->GetFloatValue(0) * VerticalRecoilModifier);
        CharacterController->AddYawInput(WeaponData->HorizontalRecoilCurve->GetFloatValue(0) * HorizontalRecoilModifier);
    }

    ShotsFired += 1;
    GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(WeaponData->RecoilCameraShake);  
}

void ASWeaponBase::RecoilRecovery()
{
    if (bShouldRecover)
    {
        // Recoil recovery
        RecoilRecoveryTimeline.PlayFromStart();
    }
}


void ASWeaponBase::Reload()
{
    
    // Casting to the game instance (which stores all the ammunition and health variables)
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

    // Changing the maximum ammunition based on if the weapon can hold a bullet in the chamber
    int Value = 0;
    if(WeaponData->bCanBeChambered)
    {
        Value = 1;
    }

    // Checking if we are not reloading, if a reloading montage exists, and if there is any point in reloading (current ammunition does not match maximum magazine capacity and there is spare ammunition to load into the gun)
    if(!bIsReloading && CharacterController->AmmoMap[(PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.AmmoType : PlayerCharacter->SecondaryWeaponCacheMap.AmmoType)] > 0 && (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) != (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipCapacity : PlayerCharacter->SecondaryWeaponCacheMap.ClipCapacity) + Value)
    {
         // Differentiating between having no ammunition in the magazine (having to chamber a round after reloading) or not, and playing an animation relevant to that
        if ((PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) <= 0 && WeaponData->EmptyPlayerReload)
        {
            if (WeaponData->bHasAttachments)
            {
                MagazineAttachment->PlayAnimation(WeaponData->EmptyWeaponReload, false);
            }
            else
            {
                MeshComp->PlayAnimation(WeaponData->EmptyWeaponReload, false);
            }
            
            AnimTime = PlayerCharacter->HandsMeshComp->GetAnimInstance()->Montage_Play(WeaponData->EmptyPlayerReload, 1.0f);
        }
        else if (WeaponData->PlayerReload)
        {
            if (WeaponData->bHasAttachments)
            {
                MagazineAttachment->PlayAnimation(WeaponData->WeaponReload, false);
            }
            else
            {
                MeshComp->PlayAnimation(WeaponData->WeaponReload, false);
            }
            AnimTime = PlayerCharacter->HandsMeshComp->GetAnimInstance()->Montage_Play(WeaponData->PlayerReload, 1.0f);
        }
        else
        {
            AnimTime = 2.0f;
        }
        
        // Printing debug strings
        if(bShowDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Reload", true);
        }
        
        // Setting variables to make sure that the player cannot fire or reload during the time that the weapon is in it's reloading animation
        bCanFire = false;
        bIsReloading = true;
        
        // Starting the timer alongside the animation of the weapon reloading, casting to UpdateAmmo when it finishes
        GetWorldTimerManager().SetTimer(ReloadingDelay, this, &ASWeaponBase::UpdateAmmo, AnimTime, false, AnimTime);
    }
    
}

void ASWeaponBase::UpdateAmmo()
{ 
    // Printing debug strings
    if(bShowDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "UpdateAmmo", true);
    }

    // Casting to the game instance (which stores all the ammunition and health variables)
    ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());
    
    // value system to reload the correct amount of bullets if the weapon is using a chambered reloading system
    int value = 0;

    // Checking to see if there is already ammunition within the gun and that this particular gun supports chambered rounds
    if ((PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) > 0 && WeaponData->bCanBeChambered)
    {
        value = 1;

        if(bShowDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Value = 1", true);
        }
    }
    
    // First, we set Temp, which keeps track of the difference between the maximum ammunition and the amount that there is currently loaded (i.e. how much ammunition we need to reload into the gun)
    const int Temp = (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipCapacity : PlayerCharacter->SecondaryWeaponCacheMap.ClipCapacity) - (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize);
    // Making sure we have enough ammunition to reload
    if (CharacterController->AmmoMap[(PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.AmmoType : PlayerCharacter->SecondaryWeaponCacheMap.AmmoType)] >= Temp + value)
    {
        // Then, we update the weapon to have full ammunition, plus the value (1 if there is a bullet in the chamber, 0 if not)
        (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) = (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipCapacity : PlayerCharacter->SecondaryWeaponCacheMap.ClipCapacity) + value;
        // Finally, we remove temp (and an extra bullet, if one is chambered) from the player's ammunition store
        CharacterController->AmmoMap[(PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.AmmoType : PlayerCharacter->SecondaryWeaponCacheMap.AmmoType)] -= (Temp + value);
    }
    // If we don't, add the remaining ammunition to the clip, and set the remaining ammunition to 0
    else
    {
        (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) = (PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) + CharacterController->AmmoMap[(PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.AmmoType : PlayerCharacter->SecondaryWeaponCacheMap.AmmoType)];
        CharacterController->AmmoMap[(PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.AmmoType : PlayerCharacter->SecondaryWeaponCacheMap.AmmoType)] = 0;
    }

    // Print debug strings
    if(bShowDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::FromInt((PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize)), true);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::FromInt(CharacterController->AmmoMap[(PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.AmmoType : PlayerCharacter->SecondaryWeaponCacheMap.AmmoType)]), true);
    }

    // Resetting bIsReloading and allowing the player to fire the gun again
    bIsReloading = false;
    EnableFire();
}


// Called every frame
void ASWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    VerticalRecoilTimeline.TickTimeline(DeltaTime);
    HorizontalRecoilTimeline.TickTimeline(DeltaTime);
    RecoilRecoveryTimeline.TickTimeline(DeltaTime);
}

// Recovering the player's recoil to the pre-fired position
void ASWeaponBase::HandleRecoveryProgress(float value) const
{
    // Getting a reference to the Character Controller
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

    // Calculating the new control rotation by interpolating between current and target 
    const FRotator NewControlRotation = FMath::Lerp(CharacterController->GetControlRotation(), ControlRotation, value);
    
    CharacterController->SetControlRotation(NewControlRotation);
}