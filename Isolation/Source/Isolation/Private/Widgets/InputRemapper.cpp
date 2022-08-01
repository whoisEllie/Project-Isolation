// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InputRemapper.h"
#include "FPSCharacter.h"
#include "func_lib/WidgetHelpers.h"
#include "Kismet/GameplayStatics.h"

void UInputRemapper::NativePreConstruct()
{
	Super::NativeConstruct();

	InputName->SetText(InputDisplayName);
	
	PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void UInputRemapper::ClearActionMappings()
{
	PlayerCharacter->GetBaseMappingContext()->UnmapAction(InputAction);
}

void UInputRemapper::RemapKey(FInputChord InSelectedKey)
{
	PlayerCharacter->GetBaseMappingContext()->MapKey(InputAction, InSelectedKey.Key);
}

void UInputRemapper::SelectedKeyChanged()
{
	UUserWidget* ParentCanvas = WidgetHelpers::GetParentCanvas(Cast<UWidget>(this));
	USettingsWidget* SettingsWidget = Cast<USettingsWidget>(ParentCanvas);
	SettingsWidget->SwitchKeySelectPopupVisibility();
}
