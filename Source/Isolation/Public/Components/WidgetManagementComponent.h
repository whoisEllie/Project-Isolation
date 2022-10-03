// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/ActorComponent.h"
#include "Widgets/HUDWidget.h"
#include "Widgets/PauseWidget.h"
#include "Widgets/SettingsWidget.h"
#include "WidgetManagementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ISOLATION_API UWidgetManagementComponent final : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Called to bind functionality to input */
	void SetupInputComponent(class UEnhancedInputComponent* PlayerInputComponent);

	/** Returns a reference to the player's heads up display */
	USHUDWidget* GetPlayerHud() const { return PlayerHudWidget; }

	/** Draws the settings menu to the screen */
	UFUNCTION(BlueprintCallable)
	void CreateSettingsMenu();

	/** The input action for pausing the game */
	UPROPERTY()
	UInputAction* PauseAction;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	/** Updates widgets */
	void ManageOnScreenWidgets();

	/** The HUD widget blueprint to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<USHUDWidget> HUDWidget;

	/** The pause widget blueprint to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<UPauseWidget> PauseWidget;

	/** The settings widget blueprint to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "UI | Widget Defaults")
	TSubclassOf<USettingsWidget> SettingsWidget;

	/** A reference to the instance of the player's main HUD widget */
	UPROPERTY()
	USHUDWidget* PlayerHudWidget;

	/** A reference to the instance of the player's pause widget */
	UPROPERTY()
	UPauseWidget* PlayerPauseWidget;

	/** A reference to the instance of the player's settings widget */
	UPROPERTY()
	USettingsWidget* PlayerSettingsWidget;

	/** The current insanced widget visible on screen */
	UPROPERTY()
	UUserWidget* CurrentWidget;
};
