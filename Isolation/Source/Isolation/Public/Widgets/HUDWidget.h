// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ISOLATION_API USHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	void ReceiveTooltipInput(FText MainText, FText DescriptionText);

	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* TooltipSlideIn;

	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* TooltipSlideOut;

	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* RepairKitSlideIn;
	
	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* RepairKitSlideOut;

	UPROPERTY(Transient, meta = ( BindWidget ))
	UTextBlock* TooltipTitle;

	UPROPERTY(Transient, meta = ( BindWidget ))
	URichTextBlock* TooltipDescription;

	FText LocalMainText;
	
	FText LocalDescriptionText;
	
	float TooltipDisplayTime;

	bool TooltipVisible = false;

	void ShowRepairKitCount();

private:

	FTimerHandle TooltipDelayTimerHandle;

	FTimerHandle RepairKitTimerHandle;

	void ShowToolTip();
	
	void HideTooltip();

	void HideRapirKitCount();

	float ReadTime(FString Text);
};
