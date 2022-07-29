// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SettingsWidget.h"

#include "FPSCharacter.h"
#include "Kismet/GameplayStatics.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void USettingsWidget::ClearActionMappings(const UInputAction* InputAction) const
{
	PlayerCharacter->GetBaseMappingContext()->UnmapAction(InputAction);
}

void USettingsWidget::RemapKey(UInputAction* InputAction, FKey Key)
{
	PlayerCharacter->GetBaseMappingContext()->MapKey(InputAction, Key);
}