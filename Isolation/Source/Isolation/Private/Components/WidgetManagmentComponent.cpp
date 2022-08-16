// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WidgetManagmentComponent.h"
#include "EnhancedInputComponent.h"
#include "FPSCharacter.h"
#include "FPSCharacterController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UWidgetManagmentComponent::UWidgetManagmentComponent()
{
}


// Called when the game starts
void UWidgetManagmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialising our widgets, and adding the HUD widget to the screen
	if (IsValid(HUDWidget))
	{
		PlayerHudWidget = Cast<USHUDWidget>(CreateWidget(GetWorld(), HUDWidget));
		if (PlayerHudWidget != nullptr)
		{
			PlayerHudWidget->AddToViewport();
		}
	}
	if (IsValid(PauseWidget))
	{
		PlayerPauseWidget = Cast<UPauseWidget>(CreateWidget(GetWorld(), PauseWidget));
	}
	if (IsValid(SettingsWidget))
	{
		PlayerSettingsWidget = Cast<USettingsWidget>(CreateWidget(GetWorld(), SettingsWidget));
	}
	
}

void UWidgetManagmentComponent::ManageOnScreenWidgets()
{
	if (const AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetOwner()))
	{
		ASCharacterController* CharacterController = Cast<ASCharacterController>(FPSCharacter->GetController());
    
		if (CurrentWidget != nullptr)
		{
			if (CurrentWidget == PlayerPauseWidget)
			{
				PlayerPauseWidget->RemoveFromParent();
				PlayerHudWidget->AddToViewport();
				CharacterController->SetInputMode(FInputModeGameOnly());
				CharacterController->SetShowMouseCursor(false);
				UGameplayStatics::SetGamePaused(GetWorld(), false);
				CurrentWidget = nullptr;
			}
			else
			{
				PlayerSettingsWidget->RemoveFromParent();
				PlayerPauseWidget->AddToViewport();
				CurrentWidget = PlayerPauseWidget;
			}
		}
		else
		{
			PlayerHudWidget->RemoveFromParent();
			PlayerPauseWidget->AddToViewport();
			CurrentWidget = PlayerPauseWidget;
			CharacterController->SetInputMode(FInputModeGameAndUI());
			CharacterController->SetShowMouseCursor(true);
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}

void UWidgetManagmentComponent::CreateSettingsMenu()
{
	if (PlayerPauseWidget)
	{
		PlayerPauseWidget->RemoveFromParent();
		if (PlayerSettingsWidget)
		{
			PlayerSettingsWidget->AddToViewport();
			CurrentWidget = PlayerSettingsWidget;
		}
	}
}

void UWidgetManagmentComponent::SetupInputComponent(UEnhancedInputComponent* PlayerInputComponent)
{
	if (PauseAction)
	{
		// Pausing the game
		PlayerInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &UWidgetManagmentComponent::ManageOnScreenWidgets);
	}
}