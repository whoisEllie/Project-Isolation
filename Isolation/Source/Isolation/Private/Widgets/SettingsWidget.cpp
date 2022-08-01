// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SettingsWidget.h"

void USettingsWidget::SwitchKeySelectPopupVisibility()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Swapping visibility"));
	
	switch (PressAnyKeyBlur->GetVisibility())
	{
	case ESlateVisibility::Hidden:
		PressAnyKeyBlur->SetVisibility(ESlateVisibility::HitTestInvisible);
		PressAnyKeyBlur->SetIsEnabled(true);
		return;

	case ESlateVisibility::HitTestInvisible:
		PressAnyKeyBlur->SetVisibility(ESlateVisibility::Hidden);
		PressAnyKeyBlur->SetIsEnabled(false);
		return;
		
	default: return;
	}
}
