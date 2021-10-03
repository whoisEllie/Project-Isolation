// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "SIsolationGameInstance.h"

// Sets default values
ASWeaponBase::ASWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Creating our weapon's skeletal mesh, telling it to not cast shadows and finally setting it as the root of the actor
    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("meshComp"));
    meshComp->CastShadow = false;
    RootComponent = meshComp;
    
    // Default value allowing the weapon to be fired
    bCanFire = true;
}

// Called when the game starts or when spawned
void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeaponBase::StartFire()
{
    // If the weapon can be fired (i.e. not reloading, for example)
    if (bCanFire)
    {
        // sets a timer for firing the weapon - if bAutomaticFire is true then this timer will repeat until cleared by StopFire(), leading to fully automatic fire
        GetWorldTimerManager().SetTimer(shotDelay, this, &ASWeaponBase::Fire, rateOfFire, bAutomaticFire, 0.0f);
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
    GetWorldTimerManager().ClearTimer(shotDelay);
}

void ASWeaponBase::Fire()
{
    // Casting to the game instance (which stores all the ammunition and health variables)
    USIsolationGameInstance* GameInstanceRef = Cast<USIsolationGameInstance>(GEngine->GetWorld()->GetGameInstance());
    
    // Allowing the gun to fire if it has ammunition, is not reloading and the bCanFire variable is true
    if(bCanFire && GameInstanceRef->weaponParameters[0].clipSize > 0 && !bIsReloading)
    {

        // Setting up the parameters we need to do a line trace from the muzzle of the gun and calculating the start and end points of the ray trace
        traceStart = meshComp->GetSocketLocation(muzzleSocketName);
		traceStartRotation = meshComp->GetSocketRotation(muzzleSocketName);
		traceDirection = traceStartRotation.Vector();
		traceEnd = traceStart + (traceDirection * lengthMultiplier);

        // Determining the spawn parameters for the line trace - forcing it to always spawn
        FActorSpawnParameters spawnParams;
        spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Subtracting from the ammunition count of the weapon
        GameInstanceRef->weaponParameters[0].clipSize -= 1;

        // Drawing a line trace based on the parameters calculated previously 
        if(GetWorld()->LineTraceSingleByChannel(hit, traceStart, traceEnd, ECC_GameTraceChannel1, queryParams))
        {
            // Resetting finalDamage
            finalDamage = 0.0f;

            // Setting finalDamage based on the type of surface hit
            if (hit.PhysMaterial.Get() == normalDamageSurface)
            {
                finalDamage = baseDamage;
            }
            if (hit.PhysMaterial.Get() == headshotDamageSurface)
            {
                finalDamage = baseDamage * headshotMultiplier;
            }

            AActor* hitActor = hit.GetActor();

            // Applying the previously set damage to the hit actor
            UGameplayStatics::ApplyPointDamage(hitActor, finalDamage, traceDirection, hit, GetInstigatorController(), this, damageType);
        }

        // Selecting the hit effect based on the hit physical surface material (hit.PhysMaterial.Get()) and spawning it (Niagara)

        if (hit.PhysMaterial.Get() == normalDamageSurface || hit.PhysMaterial.Get() == headshotDamageSurface)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), enemyHitEffect, hit.ImpactPoint, hit.ImpactNormal.Rotation());
        }
        else if (hit.PhysMaterial.Get() == groundSurface)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), groundHitEffect, hit.ImpactPoint, hit.ImpactNormal.Rotation());
        }
        else if (hit.PhysMaterial.Get() == rockSurface)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), rockHitEffect, hit.ImpactPoint, hit.ImpactNormal.Rotation());
        }
        else
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), defaultHitEffect, hit.ImpactPoint, hit.ImpactNormal.Rotation());
        }

        // If a fire montage is specified, play it
        if (fireMontage)
        {
            animTime = meshComp->GetAnimInstance()->Montage_Play(fireMontage, 1.0f);
        }

        // If we wait for an animation (and fireMontage exists, since we need it to determine the length of the timer)
        if (bWaitForAnim && fireMontage)
        {
            bCanFire = false;

            GetWorldTimerManager().SetTimer(animationWaitDelay, this, &ASWeaponBase::EnableFire, animTime, false, animTime); // 
        }
    }
}

void ASWeaponBase::Reload()
{
    // Casting to the game instance (which stores all the ammunition and health variables)
    USIsolationGameInstance* GameInstanceRef = Cast<USIsolationGameInstance>(GEngine->GetWorld()->GetGameInstance());
    
    // Checking if we are not reloading, if a reloading montage exists, and if there is any point in reloading (current ammunition does not match maximum magazine capacity and there is spare ammunition to load into the gun)
    if(!bIsReloading && reloadMontage && GameInstanceRef->ammoMap[GameInstanceRef->weaponParameters[0].ammoType] > 0 && GameInstanceRef->weaponParameters[0].clipSize != GameInstanceRef->weaponParameters[0].clipCapacity)
    {
         // Differenciating between having no ammunition in the magazine (having to chamber a round after reloading) or not, and playing an animation relevant to that
        if (GameInstanceRef->weaponParameters[0].clipSize <= 0)
        {
            animTime = meshComp->GetAnimInstance()->Montage_Play(emptyReloadMontage, 1.0f);
        }
        else
        {
            animTime = meshComp->GetAnimInstance()->Montage_Play(reloadMontage, 1.0f);
        }
        
        // Setting variables to make sure that the player cannot fire or reload during the time that the weapon is in it's reloading animation
        bCanFire = false;
        bIsReloading = true;
        // Starting the timer alongside the animation of the weapon reloading, casting to UpdateAmmo when it finishes
        GetWorldTimerManager().SetTimer(reloadingDelay, this, &ASWeaponBase::UpdateAmmo, animTime, false, animTime);
    }
}

void ASWeaponBase::UpdateAmmo()
{
    // Casting to the game instance (which stores all the ammunition and health variables)
    USIsolationGameInstance* GameInstanceRef = Cast<USIsolationGameInstance>(GEngine->GetWorld()->GetGameInstance());
    
    int value = 0;

    // Checking to see if there is already ammunition within the gun and that this particular gun supports chambered rounds
    if (GameInstanceRef->weaponParameters[0].clipSize > 0 && bCanBeChambered)
    {
        value = 1;
    }
    
    // First, we set temp, which keeps track of the difference between the maximum ammunition and the amount that there is currently loaded (i.e. how much ammunition we need to reload into the gun)
    int temp = GameInstanceRef->weaponParameters[0].clipCapacity - GameInstanceRef->weaponParameters[0].clipSize;
    // Making sure we have enough ammunition to reload
    if (GameInstanceRef->ammoMap[GameInstanceRef->weaponParameters[0].ammoType] >= temp + value)
    {
        // Then, we update the weapon to have full ammunition, plus the value (1 if there is a bullet in the chamber, 0 if not)
        GameInstanceRef->weaponParameters[0].clipSize = GameInstanceRef->weaponParameters[0].clipCapacity + value;
        // Finally, we remove temp (and an extra bullet, if one is chambered) from the player's ammunition store
        GameInstanceRef->ammoMap[GameInstanceRef->weaponParameters[0].ammoType] -= (temp + value);
    }
    // If we don't, add the remaining ammunition to the clip, and set the remaining ammunition to 0
    else
    {
        GameInstanceRef->weaponParameters[0].clipSize = GameInstanceRef->weaponParameters[0].clipSize + GameInstanceRef->ammoMap[GameInstanceRef->weaponParameters[0].ammoType];
        GameInstanceRef->ammoMap[GameInstanceRef->weaponParameters[0].ammoType] = 0;
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

