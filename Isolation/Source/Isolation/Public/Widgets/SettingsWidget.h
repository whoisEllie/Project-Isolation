// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/BackgroundBlur.h"
#include "SettingsWidget.generated.h"

UCLASS()
class ISOLATION_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** A reference to the blur applied to the screen when updating a key binding */
	UPROPERTY(Transient, meta = ( BindWidget ))
	UBackgroundBlur* PressAnyKeyBlur;

	/** Called to swap the visibility of the popup window */
	UFUNCTION(BlueprintCallable)
	void SwitchKeySelectPopupVisibility() const;
};
