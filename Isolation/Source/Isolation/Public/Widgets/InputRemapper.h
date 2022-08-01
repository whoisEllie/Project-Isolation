// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputAction.h"
#include "InputRemapper.generated.h"

class UButton;
class UInputKeySelector;
class UTextBlock;
class URichTextBlock;
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

	void UpdateMappings() const;

	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;
	
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

	UPROPERTY(Transient, meta = ( BindWidget ))
	URichTextBlock* InputDisplayTextBlock;
	
	UPROPERTY()
	AFPSCharacter* PlayerCharacter;
};