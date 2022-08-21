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

	/** Clear all the input action mappings associated with this widget */
	UFUNCTION(BlueprintCallable)
	void ClearActionMappings() const;

	/** Remap the input action associated with this widget
	 *	@param InSelectedKey The new key to map
	 */
	UFUNCTION(BlueprintCallable)
	void RemapKey(FInputChord InSelectedKey) const;

	/** Update the visibility of the overlay in the parent widget when the InputKeySelector's status is changed */
	UFUNCTION(BlueprintCallable)
	void SelectedKeyChanged();

	/** Updates the mappings displayed on the screen from the current mapping context */
	void UpdateMappings() const;

	/** Called when the widget is edited in the editor */
	virtual void NativePreConstruct() override;

	/** Called when the widget is added to the screen */
	virtual void NativeConstruct() override;

	/** The input action associated with this widget */
	UPROPERTY(EditInstanceOnly, Category = "Settings")
	UInputAction* InputAction;

	/** The display name of the given input (what to show on screen) */
	UPROPERTY(EditInstanceOnly, Category = "Settings")
	FText InputDisplayName;

private:

	/** The text block that holds the name of the input */
	UPROPERTY(Transient, meta = ( BindWidget ))
	UTextBlock* InputName;

	/** The rich text block that holds the current keymappings for this input */
	UPROPERTY(Transient, meta = ( BindWidget ))
	URichTextBlock* InputDisplayTextBlock;

	/** A reference to the player character */
	UPROPERTY()
	AFPSCharacter* PlayerCharacter;
};