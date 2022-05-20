// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "SHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ISOLATION_API USHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	void ReceiveInput(FText MainText, FText DescriptionText);

	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* TooltipSlideIn;

	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* TooltipSlideOut;

	UPROPERTY(Transient, meta = ( BindWidget ))
	UTextBlock* TooltipTitle;

	UPROPERTY(Transient, meta = ( BindWidget ))
	UMultiLineEditableTextBox* TooltipDescription;

	FText LocalMainText;
	
	FText LocalDescriptionText;
	
	float LocalTime;

	bool TooltipVisible = false;

private:

	FTimerHandle DelayTimerHandle;

	void ShowToolTip();
	
	void HideTooltip();

	float ReadTime(FString Text);
};
