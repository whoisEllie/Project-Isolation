// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SettingsWidget.h"

void USettingsWidget::SwitchKeySelectPopupVisibility() const
{	
	switch (PressAnyKeyBlur->GetVisibility())
	{
		case ESlateVisibility::Hidden:
			// We make sure that ESlateVisibility is set to HitTestInvisible, so that the UInputKeySelector can be
			// interacted with
			PressAnyKeyBlur->SetVisibility(ESlateVisibility::HitTestInvisible);
			PressAnyKeyBlur->SetIsEnabled(true);
			return;

		case ESlateVisibility::HitTestInvisible:
			PressAnyKeyBlur->SetVisibility(ESlateVisibility::Hidden);
			PressAnyKeyBlur->SetIsEnabled(false);
			
		default: ;
	}
}
