// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "SIsolationGameInstance.h"

// Sets default values
ASWeaponBase::ASWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("meshComp"));
    meshComp->CastShadow = false;
    RootComponent = meshComp;
    
    canFire = true;
}

// Called when the game starts or when spawned
void ASWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeaponBase::StartFire()
{
    if (canFire)
    {
        GetWorldTimerManager->SetTimer(timerHandleShotDelay, )
    }
    
}

void ASWeaponBase::StopFire()
{

}

void ASWeaponBase::Fire()
{
    USIsolationGameInstance* GameInstanceRef = Cast<USIsolationGameInstance>(GEngine->GetWorld()->GetGameInstance());
    
    if(canFire)
    {
        if (GameInstanceRef->weaponParameters[0].clipSize > 0)
        {
            //Setting up the parameters we need to do a line trace from the muzzle of the gun and calculating the start and end points of the ray trace
            traceStart = meshComp->GetSocketLocation(muzzleSocketName);
			traceStartRotation = meshComp->GetSocketRotation(muzzleSocketName);
			traceDirection = traceStartRotation.Vector();
			traceEnd = traceStart + (traceDirection * lengthMultiplier);

            FActorSpawnParameters spawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            GameInstanceRef->weaponParameters[0].clipSize -= 1;

            if(GetWorld->LineTraceSingleByChannel(hit, traceStart, traceEnd, ECC_GameTraceChannel1, queryParams))
            {
                finalDamage = 0.0f;

                if (hit.PhysMaterial.Get() == normalDamageSurface)
                {
                    finalDamage = baseDamage;
                }
                if (hit.PhysMaterial.Get() == headshotDamageSurface)
                {
                    finalDamage = baseDamage * headshotMultiplier;
                }

                AActor* hitActor = hit.GetActor();

                UGameplayStatics::ApplyPointDamage(hitActor, finalDamage, traceDirection, hit, GetInstigatorController(), this, damageType);
            }

        }
    }
}

void ASWeaponBase::Reload()
{

}

// Called every frame
void ASWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

