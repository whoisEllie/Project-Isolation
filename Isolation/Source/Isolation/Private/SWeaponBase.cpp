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

    WeaponData = WeaponDataTable->FindRow<FWeaponData>(FName("Sten"), FString("Sten Gun"), true); // Make sure to modularize this!!
    bSilenced = WeaponData->bIsSilenced;


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
    for (FName RowName : AttachmentsArray)
    {
        
        AttachmentData = AttachmentsDataTable->FindRow<FAttachmentData>(RowName, RowName.ToString(), true);

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
                SocketOverride = AttachmentData->MuzzleLocationOverride;
                ParticleSocketOverride = AttachmentData->ParticleSpawnLocationOverride;
                bSilenced = AttachmentData->bSilenced;
            }
            else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
            {
                MagazineAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                SoundOverride = AttachmentData->FiringSoundOverride;
                WeaponData->RateOfFire = AttachmentData->FireRate;
                WeaponData->bAutomaticFire = AttachmentData->AutomaticFire;
                VerticalRecoilCurve = AttachmentData->VerticalRecoilCurve;
                HorizontalRecoilCurve = AttachmentData->HorizontalRecoilCurve;
                RecoilCameraShake = AttachmentData->RecoilCameraShake;
                bIsShotgun = AttachmentData->bIsShotgun;
                ShotgunLength = AttachmentData->ShotgunRange;
                ShotgunPellets = AttachmentData->ShotgunPellets;
                EmptyWeaponReload = AttachmentData->EmptyWeaponReload;
                WeaponReload = AttachmentData->WeaponReload;
                EmptyPlayerReload = AttachmentData->EmptyPlayerReload;
                PlayerReload = AttachmentData->PlayerReload;
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
                if (AttachmentData->BS_ADS_Walk)
                {
                    ADSWalkBlendSpace = AttachmentData->BS_ADS_Walk;
                }
                if (AttachmentData->Anim_Idle)
                {
                    Anim_Idle = AttachmentData->Anim_Idle;
                }
                if (AttachmentData->Anim_Sprint)
                {
                    Anim_Sprint = AttachmentData->Anim_Sprint;
                }
                if (AttachmentData->Anim_ADS_Idle)
                {
                    Anim_ADS_Idle = AttachmentData->Anim_ADS_Idle;
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

        StartRecoil();
    }
    
}

void ASWeaponBase::StartRecoil()
{
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());
    

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
    const float test_var = USWeaponStatsCalculator::GetAccuracyRating(WeaponPitchModifier, WeaponData->WeaponPitchVariation, WeaponYawModifier, WeaponData->WeaponYawVariation);

    
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

        const int NumberOfShots = bIsShotgun? ShotgunPellets : 1;
        for (int i = 0; i < NumberOfShots; i++)
        {
            // Setting up the parameters we need to do a line trace from the muzzle of the gun and calculating the start and end points of the ray trace
            TraceStart = MeshComp->GetSocketLocation(WeaponData->MuzzleSocketName);
            TraceStartRotation = MeshComp->GetSocketRotation(WeaponData->MuzzleSocketName);
            if (SocketOverride != "")
            {
                TraceStart = BarrelAttachment->GetSocketLocation(SocketOverride);
                TraceStartRotation = BarrelAttachment->GetSocketRotation(SocketOverride);
            }
            TraceStartRotation.Pitch += FMath::FRandRange(-(WeaponData->WeaponPitchVariation + WeaponPitchModifier), WeaponData->WeaponPitchVariation + WeaponPitchModifier);
            TraceStartRotation.Yaw += FMath::FRandRange(-(WeaponData->WeaponYawVariation + WeaponYawModifier), WeaponData->WeaponYawVariation + WeaponYawModifier);
            TraceDirection = TraceStartRotation.Vector();
            TraceEnd = TraceStart + (TraceDirection * (bIsShotgun? ShotgunLength : WeaponData->LengthMultiplier));

            // Applying Recoil to the weapon
            Recoil();

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

        if(bSilenced)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SilencedOverride? SilencedOverride : WeaponData->SilencedSound, TraceStart);
        }
        else
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundOverride? SoundOverride : WeaponData->FireSound, TraceStart);
        }

        // Spawning the firing particle effect
        if (ParticleSocketOverride != "")
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponData->MuzzleFlash, BarrelAttachment, ParticleSocketOverride, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true, true);
        }
        else
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponData->MuzzleFlash, MeshComp, WeaponData->ParticleSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true, true);
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
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->EmptyFireSound, MeshComp->GetSocketLocation(WeaponData->MuzzleSocketName));
        // Clearing the ShotDelay timer so that we don't have a constant ticking when the player has no ammo, just a single click
        GetWorldTimerManager().ClearTimer(ShotDelay);

        RecoilRecovery();
    }
    
}

void ASWeaponBase::Recoil() const
{
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());
    if (WeaponData->bAutomaticFire && CharacterController)
    {
        CharacterController->AddPitchInput(VerticalRecoilCurve->GetFloatValue(VerticalRecoilTimeline.GetPlaybackPosition()) * VerticalRecoilModifier);
        CharacterController->AddYawInput(HorizontalRecoilCurve->GetFloatValue(HorizontalRecoilTimeline.GetPlaybackPosition()) * HorizontalRecoilModifier);
    }
    else
    {
        CharacterController->AddPitchInput(VerticalRecoilCurve->GetFloatValue(0) * VerticalRecoilModifier);
        CharacterController->AddYawInput(HorizontalRecoilCurve->GetFloatValue(0) * HorizontalRecoilModifier);
    }

    GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(RecoilCameraShake);  

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
    ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
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
        if ((PlayerCharacter->bIsPrimary? PlayerCharacter->PrimaryWeaponCacheMap.ClipSize : PlayerCharacter->SecondaryWeaponCacheMap.ClipSize) <= 0 && EmptyPlayerReload)
        {
            // TODO: Figure out magazine reload animations 
            //MagazineAttachment->PlayAnimation(EmptyWeaponReload, false);
            AnimTime = PlayerCharacter->HandsMeshComp->GetAnimInstance()->Montage_Play(EmptyPlayerReload, 1.0f);
        }
        else if (PlayerReload)
        {
            // TODO: Figure out magazine reload animations
            //MagazineAttachment->PlayAnimation(WeaponReload, false);
            AnimTime = PlayerCharacter->HandsMeshComp->GetAnimInstance()->Montage_Play(PlayerReload, 1.0f);
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


void ASWeaponBase::HandleVerticalRecoilProgress(float value) const
{
    if (bShowDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::SanitizeFloat(value));
    }
}

void ASWeaponBase::HandleHorizontalRecoilProgress(float value) const
{
    if (bShowDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::SanitizeFloat(value));
    }
}


void ASWeaponBase::HandleRecoveryProgress(float value) const
{
    const ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());

    const FRotator NewControlRotation = FMath::Lerp(CharacterController->GetControlRotation(), ControlRotation, value);
    
    CharacterController->SetControlRotation(NewControlRotation);
}