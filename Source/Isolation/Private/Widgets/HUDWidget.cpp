// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/HUDWidget.h"
#include "FPSCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "func_lib/FInputHelper.h"
#include "Kismet/GameplayStatics.h"

void USHUDWidget::DisplayToolTip(const FText MainText, const FText DescriptionText)
{
	LocalMainText = MainText;
	LocalDescriptionText = DescriptionText;
	TooltipDisplayTime = ReadTime(DescriptionText.ToString());

	if (TooltipVisible)
	{
		// If the tooltip is visible, we need to hide it before we can show the new tooltip
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
	GetWorld()->GetTimerManager().SetTimer(RepairKitTimerHandle, this, &USHUDWidget::HideRepairKitCount, 5.0f, false, 5.0f);
}

void USHUDWidget::UpdateInteractKeyIndicator() const
{
	const AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	TArray<FEnhancedActionKeyMapping> Mappings = PlayerCharacter->GetBaseMappingContext()->GetMappings();

	FString InputDisplayText = "";

	// Iterating through all the action mappings and adding the rich text images for the selected input to the screen 
	for (FEnhancedActionKeyMapping Mapping : Mappings)
	{
		if (Mapping.Action == InteractAction)
		{
			InputDisplayText.Append("<img id=\"" + FInputHelper::KeyConversionMap[Mapping.Key.GetDisplayName().ToString()] + "\"/> ");
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

void USHUDWidget::HideRepairKitCount()
{
	PlayAnimation(RepairKitSlideOut);
}

float USHUDWidget::ReadTime(const FString Text) const
{
	// Dividing length of text by average word length of 4.7 letters
	// Multiplying by average word per second read speed
	// Adding offset
	return Text.Len()/4.7f*60.0f/250.0f + ReadTimeOffset;
}
