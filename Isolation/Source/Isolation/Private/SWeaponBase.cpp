// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponBase.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "SCharacterController.h"
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
}

// Called when the game starts or when spawned
void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();

    ReferenceWeapon = this->GetClass();

    //Sets the default values for our trace query
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

    WeaponData = WeaponDataTable->FindRow<FWeaponData>(FName("Sten"), FString("Sten Gun"), true);
    bSilenced = WeaponData->bIsSilenced;
    SpawnAttachments(AttachmentNameArray);
}

void ASWeaponBase::SpawnAttachments(TArray<FName> AttachmentsArray)
{
    ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());
    
    for (FName RowName : AttachmentsArray)
    {
        
        AttachmentData = AttachmentsDataTable->FindRow<FAttachmentData>(RowName, RowName.ToString(), true);

        if (AttachmentData)
        {
            DamageModifier += AttachmentData->BaseDamageImpact;
            WeaponPitchModifier += AttachmentData->WeaponPitchVariationImpact;
            WeaponYawModifier += AttachmentData->WeaponYawVariationImpact;

            if (AttachmentData->AttachmentType == EAttachmentType::Barrel)
            {
                BarrelAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                SocketOverride = AttachmentData->MuzzleLocationOverride;
                ParticleSocketOverride = AttachmentData->ParticleSpawnLocationOverride;
                if (AttachmentData->bSilenced)
                {
                    bSilenced = true;
                }
            }
            else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
            {
                MagazineAttachment->SetSkeletalMesh(AttachmentData->AttachmentMesh);
                SoundOverride = AttachmentData->FiringSoundOverride;
                CharacterController->WeaponParameters[ReferenceWeapon].AmmoType = AttachmentData->AmmoToUse;
                CharacterController->WeaponParameters[ReferenceWeapon].ClipCapacity = AttachmentData->ClipCapacity;
                CharacterController->WeaponParameters[ReferenceWeapon].ClipSize = AttachmentData->ClipSize;
                CharacterController->WeaponParameters[ReferenceWeapon].WeaponHealth = AttachmentData->WeaponHealth;
                WeaponData->RateOfFire = AttachmentData->FireRate;
                WeaponData->bAutomaticFire = AttachmentData->AutomaticFire;
                
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
}

void ASWeaponBase::Fire()
{ 
    // Casting to the game instance (which stores all the ammunition and health variables)
    ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    ASCharacterController* CharacterController = Cast<ASCharacterController>(PlayerCharacter->GetController());
    
    // Allowing the gun to fire if it has ammunition, is not reloading and the bCanFire variable is true
    if(bCanFire && CharacterController->WeaponParameters[ReferenceWeapon].ClipSize > 0 && !bIsReloading)
    {

        // Printing debug strings
        if(bShowDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Fire", true);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::FromInt(CharacterController->WeaponParameters[ReferenceWeapon].ClipSize), true);
        }
        

        // Subtracting from the ammunition count of the weapon
        CharacterController->WeaponParameters[ReferenceWeapon].ClipSize -= 1;

        int NumberOfShots = WeaponData->bIsShotgun? WeaponData->ShotgunPelletCount : 1;
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
		    TraceEnd = TraceStart + (TraceDirection * WeaponData->LengthMultiplier);

            // Drawing debug line trace
            if (bShowDebug)
            {
                DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 10.0f, 0.0f, 2.0f);
            }

             // Drawing a line trace based on the parameters calculated previously 
            if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_GameTraceChannel1, QueryParams))
            {
                // Resetting finalDamage
                FinalDamage = 0.0f;

                // Setting finalDamage based on the type of surface hit
                if (Hit.PhysMaterial.Get() == WeaponData->NormalDamageSurface)
                {
                   FinalDamage = (WeaponData->BaseDamage + DamageModifier);
                }
                if (Hit.PhysMaterial.Get() == WeaponData->HeadshotDamageSurface)
                {
                   FinalDamage = (WeaponData->BaseDamage + DamageModifier) * WeaponData->HeadshotMultiplier;
                }

                AActor* HitActor = Hit.GetActor();

                // Applying the previously set damage to the hit actor
                UGameplayStatics::ApplyPointDamage(HitActor, FinalDamage, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
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
    }
    else if (bCanFire && !bIsReloading)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->EmptyFireSound, MeshComp->GetSocketLocation(WeaponData->MuzzleSocketName));
        // Clearing the ShotDelay timer so that we don't have a constant ticking when the player has no ammo, just a single click
        GetWorldTimerManager().ClearTimer(ShotDelay);
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
    if(!bIsReloading && CharacterController->AmmoMap[CharacterController->WeaponParameters[ReferenceWeapon].AmmoType] > 0 && CharacterController->WeaponParameters[ReferenceWeapon].ClipSize != CharacterController->WeaponParameters[ReferenceWeapon].ClipCapacity + Value)
    {
         // Differentiating between having no ammunition in the magazine (having to chamber a round after reloading) or not, and playing an animation relevant to that
        if (CharacterController->WeaponParameters[ReferenceWeapon].ClipSize <= 0 && WeaponData->EmptyReloadMontage)
        {
            AnimTime = MeshComp->GetAnimInstance()->Montage_Play(WeaponData->EmptyReloadMontage, 1.0f);
        }
        else if (WeaponData->ReloadMontage)
        {
            AnimTime = MeshComp->GetAnimInstance()->Montage_Play(WeaponData->ReloadMontage, 1.0f);
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
    if (CharacterController->WeaponParameters[ReferenceWeapon].ClipSize > 0 && WeaponData->bCanBeChambered)
    {
        value = 1;

        if(bShowDebug)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Value = 1", true);
        }
    }
    
    // First, we set Temp, which keeps track of the difference between the maximum ammunition and the amount that there is currently loaded (i.e. how much ammunition we need to reload into the gun)
    int Temp = CharacterController->WeaponParameters[ReferenceWeapon].ClipCapacity - CharacterController->WeaponParameters[ReferenceWeapon].ClipSize;
    // Making sure we have enough ammunition to reload
    if (CharacterController->AmmoMap[CharacterController->WeaponParameters[ReferenceWeapon].AmmoType] >= Temp + value)
    {
        // Then, we update the weapon to have full ammunition, plus the value (1 if there is a bullet in the chamber, 0 if not)
        CharacterController->WeaponParameters[ReferenceWeapon].ClipSize = CharacterController->WeaponParameters[ReferenceWeapon].ClipCapacity + value;
        // Finally, we remove temp (and an extra bullet, if one is chambered) from the player's ammunition store
        CharacterController->AmmoMap[CharacterController->WeaponParameters[ReferenceWeapon].AmmoType] -= (Temp + value);
    }
    // If we don't, add the remaining ammunition to the clip, and set the remaining ammunition to 0
    else
    {
        CharacterController->WeaponParameters[ReferenceWeapon].ClipSize = CharacterController->WeaponParameters[ReferenceWeapon].ClipSize + CharacterController->AmmoMap[CharacterController->WeaponParameters[ReferenceWeapon].AmmoType];
        CharacterController->AmmoMap[CharacterController->WeaponParameters[ReferenceWeapon].AmmoType] = 0;
    }

    // Print debug strings
    if(bShowDebug)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::FromInt(CharacterController->WeaponParameters[ReferenceWeapon].ClipSize), true);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::FromInt(CharacterController->AmmoMap[CharacterController->WeaponParameters[ReferenceWeapon].AmmoType]), true);
    }

    // Resetting bIsReloading and allowing the player to fire the gun again
    bIsReloading = false;
    EnableFire();
}


// Called every frame
void ASWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

