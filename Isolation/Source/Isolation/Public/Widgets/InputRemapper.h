// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputAction.h"
#include "Components/Button.h"
#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
#include "InputRemapper.generated.h"

class AFPSCharacter;

UCLASS()
class ISOLATION_API UInputRemapper : public UUserWidget
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintCallable)
	void ClearActionMappings();
	
	UFUNCTION(BlueprintCallable)
	void RemapKey(FInputChord InSelectedKey);
	
	UFUNCTION(BlueprintCallable)
	void SelectedKeyChanged();

	virtual void NativePreConstruct() override;
	
	UPROPERTY(EditInstanceOnly, Category = "Settings")
	UInputAction* InputAction;

	UPROPERTY(EditInstanceOnly, Category = "Settings")
	FText InputDisplayName;

private:

	UPROPERTY(Transient, meta = ( BindWidget ))
	UTextBlock* InputName;

	UPROPERTY(Transient, meta = ( BindWidget ))
	UInputKeySelector* InputKeySelector;

	UPROPERTY(Transient, meta = ( BindWidget ))
	UButton* ClearMappingsButton;

	UPROPERTY()
	AFPSCharacter* PlayerCharacter;
	
};