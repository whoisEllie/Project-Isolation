// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUDWidget.h"

#include "FPSCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "func_lib/InputHelper.h"
#include "Kismet/GameplayStatics.h"

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

void USHUDWidget::UpdateInteractKeyIndicator()
{
	const AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	TArray<FEnhancedActionKeyMapping> Mappings = PlayerCharacter->GetBaseMappingContext()->GetMappings();

	FString InputDisplayText = "";
	
	for (FEnhancedActionKeyMapping Mapping : Mappings)
	{
		if (Mapping.Action == InteractAction)
		{
			InputDisplayText.Append("<img id=\"" + InputHelper::KeyConversionMap[Mapping.Key.GetDisplayName().ToString()] + "\"/> ");
		}
	}

	InteractRichTextBlock->SetText(FText::FromString(InputDisplayText));
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
