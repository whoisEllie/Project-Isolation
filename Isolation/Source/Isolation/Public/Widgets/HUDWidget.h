// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
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

	UPROPERTY(Transient, meta = ( BindWidget ))
	URichTextBlock* InteractRichTextBlock;

	FText LocalMainText;
	
	FText LocalDescriptionText;
	
	float TooltipDisplayTime;

	bool TooltipVisible = false;

	void ShowRepairKitCount();

	UFUNCTION(BlueprintCallable)
	void UpdateInteractKeyIndicator();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractAction;

private:

	FTimerHandle TooltipDelayTimerHandle;

	FTimerHandle RepairKitTimerHandle;

	void ShowToolTip();
	
	void HideTooltip();

	void HideRepairKitCount();

	float ReadTime(FString Text);
};
