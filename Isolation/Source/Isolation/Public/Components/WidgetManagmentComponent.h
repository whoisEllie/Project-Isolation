// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/ActorComponent.h"
#include "Widgets/HUDWidget.h"
#include "Widgets/PauseWidget.h"
#include "Widgets/SettingsWidget.h"
#include "WidgetManagmentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ISOLATION_API UWidgetManagmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWidgetManagmentComponent();
	
	/** Called to bind functionality to input */
	void SetupInputComponent(class UEnhancedInputComponent* PlayerInputComponent);

	UPROPERTY()
	UInputAction* PauseAction;

	/** Returns a reference to the player's heads up display */
	USHUDWidget* GetPlayerHud() const { return PlayerHudWidget; }

	/** Draws the settings menu to the screen */
	UFUNCTION(BlueprintCallable)
	void CreateSettingsMenu();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	/** Updates widgets */
	void ManageOnScreenWidgets();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<USHUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<UPauseWidget> PauseWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<USettingsWidget> SettingsWidget;

	/** A reference to the player's main HUD widget */
	UPROPERTY()
	USHUDWidget* PlayerHudWidget;

	/** A reference to the player's pause widget */
	UPROPERTY()
	UPauseWidget* PlayerPauseWidget;

	/** A reference to the player's settings widget */
	UPROPERTY()
	USettingsWidget* PlayerSettingsWidget;

	/** The current widget visible on screen */
	UPROPERTY()
	UUserWidget* CurrentWidget;
};
