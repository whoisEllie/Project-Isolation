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

	UPROPERTY(Transient, meta = ( BindWidget ))
	UBackgroundBlur* PressAnyKeyBlur;

	UFUNCTION(BlueprintCallable)
	void SwitchKeySelectPopupVisibility();
};
