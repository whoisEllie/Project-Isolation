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

	/** Displays a tooltip on the screen
	 *	@param MainText The title text of the tooltip
	 *	@param DescriptionText The tooltip's description
	 */
	void DisplayToolTip(FText MainText, FText DescriptionText);

	/** The animation of the tooltip popup sliding in */
	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* TooltipSlideIn;

	/** The animation of the tooltip popup sliding out */
	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* TooltipSlideOut;

	/** The animation of the repair kit popup sliding in */
	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* RepairKitSlideIn;

	/** The animation of the repair kit popup sliding out */
	UPROPERTY(Transient, meta = ( BindWidgetAnim ))
	UWidgetAnimation* RepairKitSlideOut;

	/** The title of of the tooltip popup */
	UPROPERTY(Transient, meta = ( BindWidget ))
	UTextBlock* TooltipTitle;

	/** The description of the tooltip popup */
	UPROPERTY(Transient, meta = ( BindWidget ))
	URichTextBlock* TooltipDescription;

	/** The interaction text box */
	UPROPERTY(BlueprintReadOnly, Transient, meta = ( BindWidget ))
	URichTextBlock* InteractRichTextBlock;

	/** The main text to display on the tooltip popup */
	FText LocalMainText;

	/** The description text to display on the tooltip popup */
	FText LocalDescriptionText;

	/** The time to display the tooltip */
	float TooltipDisplayTime;

	/** Whether the tooltip popup is visible or not */
	bool TooltipVisible = false;

	/** Display the amount of repair kits the player has on screen */
	void ShowRepairKitCount();

	/** Updates the key images displayed in the interaction indicator */
	UFUNCTION(BlueprintCallable)
	void UpdateInteractKeyIndicator() const;

	/** The interact input action */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractAction;

private:

	/** The timer for displaying tooltips */
	FTimerHandle TooltipDelayTimerHandle;

	/** The timer for displaying repair kit numbers */
	FTimerHandle RepairKitTimerHandle;

	/** The offset applied to the read time */
	UPROPERTY(EditDefaultsOnly, Category = "Popups")
	float ReadTimeOffset;
	
	/** Shows the tooltip popup on screen */
	void ShowToolTip();
	
	/** Hides the tooltip popup from the screen */
	void HideTooltip();

	/** Shows the repair kit popup on screen */
	void HideRepairKitCount();

	/** Calculates the amount of time to display tooltip popups on screen for by assuming an average read time per letter
	 *	@param Text The text for which to calculate the read time
	 *	@return An estimated time for how long it would take an average human to read the popup, plus an offset
	 */
	float ReadTime(FString Text) const;
};
