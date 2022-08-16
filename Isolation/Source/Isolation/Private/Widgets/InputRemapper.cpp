// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InputRemapper.h"
#include "FPSCharacter.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "func_lib/FInputHelper.h"
#include "func_lib/FWidgetHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SettingsWidget.h"

void UInputRemapper::NativePreConstruct()
{
	Super::NativeConstruct();

	InputName->SetText(InputDisplayName);
	
	PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void UInputRemapper::NativeConstruct()
{
	Super::NativeConstruct();
	
	UpdateMappings();
}

void UInputRemapper::ClearActionMappings()
{
	PlayerCharacter->GetBaseMappingContext()->UnmapAction(InputAction);

	UpdateMappings();
}

void UInputRemapper::RemapKey(const FInputChord InSelectedKey)
{
	PlayerCharacter->GetBaseMappingContext()->MapKey(InputAction, InSelectedKey.Key);

	UpdateMappings();
}

void UInputRemapper::SelectedKeyChanged()
{
	UUserWidget* ParentCanvas = FWidgetHelpers::GetParentCanvas(Cast<UWidget>(this));
	USettingsWidget* SettingsWidget = Cast<USettingsWidget>(ParentCanvas);
	SettingsWidget->SwitchKeySelectPopupVisibility();
}

void UInputRemapper::UpdateMappings() const
{
	TArray<FEnhancedActionKeyMapping> Mappings = PlayerCharacter->GetBaseMappingContext()->GetMappings();

	FString InputDisplayText = "";
	
	for (FEnhancedActionKeyMapping Mapping : Mappings)
	{
		if (Mapping.Action == InputAction)
		{
			InputDisplayText.Append("<img id=\"" + FInputHelper::KeyConversionMap[Mapping.Key.GetDisplayName().ToString()] + "\"/> ");
		}
	}

	InputDisplayTextBlock->SetText(FText::FromString(InputDisplayText));
}
