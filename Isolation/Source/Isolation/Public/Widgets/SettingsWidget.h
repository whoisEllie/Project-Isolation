// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class AFPSCharacter;

UCLASS()
class ISOLATION_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

	UPROPERTY()
	AFPSCharacter* PlayerCharacter;
	
	UFUNCTION(BlueprintCallable)
	void ClearActionMappings(const UInputAction* InputAction) const;

	UFUNCTION(BlueprintCallable)
	void RemapKey(UInputAction* InputAction, FKey Key);
};
