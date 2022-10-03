// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GlobalWeaponParamsUpdater.h"
#include "Components/BoxComponent.h"
#include "FPSCharacter.h"

// Sets default values
AGlobalWeaponParamsUpdater::AGlobalWeaponParamsUpdater()
{
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
}

void AGlobalWeaponParamsUpdater::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AGlobalWeaponParamsUpdater::UpdateMasterGlobalCombatParameters);
}

void AGlobalWeaponParamsUpdater::UpdateMasterGlobalCombatParameters(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	
	const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(OtherActor);

	if (FPSCharacter)
	{
		UAIManager* AIManagerSubsystem = GetWorld()->GetSubsystem<UAIManager>();
		if (AIManagerSubsystem)
		{
			AIManagerSubsystem->UpdateGlobalCombatParameters(GlobalCombatParametersOverride);
		}
	}
}
