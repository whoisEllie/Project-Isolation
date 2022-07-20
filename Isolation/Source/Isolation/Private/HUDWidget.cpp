// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDWidget.h"
#include "TimerManager.h"
#include "Engine/World.h"

void USHUDWidget::ReceiveTooltipInput(FText MainText, FText DescriptionText)
{
	LocalMainText = MainText;
	LocalDescriptionText = DescriptionText;
	TooltipDisplayTime = ReadTime(DescriptionText.ToString()) + 2.0f;
	
	if (TooltipVisible)
	{
		PlayAnimation(TooltipSlideOut);
		GetWorld()->GetTimerManager().SetTimer(TooltipDelayTimerHandle, this, &USHUDWidget::ShowToolTip, 0.35f, false, 0.35f);
	}
	else
	{
		ShowToolTip();
	}
}

void USHUDWidget::ShowRepairKitCount()
{
	PlayAnimation(RepairKitSlideIn);
	GetWorld()->GetTimerManager().SetTimer(RepairKitTimerHandle, this, &USHUDWidget::HideRapirKitCount, 5.0f, false, 5.0f);

}

void USHUDWidget::ShowToolTip()
{
	TooltipTitle->SetText(this->LocalMainText);
	TooltipDescription->SetText(this->LocalDescriptionText);
	
	PlayAnimation(TooltipSlideIn);

	GetWorld()->GetTimerManager().SetTimer(TooltipDelayTimerHandle, this, &USHUDWidget::HideTooltip, TooltipDisplayTime, false, TooltipDisplayTime);

	TooltipVisible = true;
}

void USHUDWidget::HideTooltip()
{
	PlayAnimation(TooltipSlideOut);
	TooltipVisible = false;
}

void USHUDWidget::HideRapirKitCount()
{
	PlayAnimation(RepairKitSlideOut);
}

float USHUDWidget::ReadTime(FString Text)
{
	return ((Text.Len()/4.7f)*60.0f)/250.0f;
}
