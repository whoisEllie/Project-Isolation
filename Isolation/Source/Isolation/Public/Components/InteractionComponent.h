// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ISOLATION_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

	/** Returns the display text of the current interactable object that the player is looking at */
	UFUNCTION(BlueprintCallable)
	FText& GetInteractText() { return InteractText; }

	/** Returns the result of the interaction trace, which is true if the object that we are looking at is able to be
 *  interacted with */
	UFUNCTION(BlueprintCallable)
	bool CanInteract() const { return bCanInteract; }

	/** Returns true if the interaction trace is hitting a weapon pickup */
	bool InteractionIsWeapon() const { return bInteractionIsWeapon; }

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/** Called to bind functionality to input */
	void SetupInputComponent(class UEnhancedInputComponent* PlayerInputComponent);

	UPROPERTY()
	UInputAction* WorldInteractAction;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	
	/** Interaction with the world using SInteractInterface */
	void WorldInteract();

	/** Displaying the indicator for interaction */
	void InteractionIndicator();
	
	/** The current message to be displayed above the screen (if any) */
	UPROPERTY()
	FText InteractText;

	/** Hit result for line trace */
	FHitResult InteractionHit;

	/** The maximum distance in unreal units at which the player can interact with an object */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractDistance = 400.0f;
	
	UPROPERTY()
	UCameraComponent* CameraComponent;

	/** Whether the object we are looking at is one we are able to interact with (used for UI) */
	bool bCanInteract;
	
	/** Whether the interaction object the character is looking at is a weapon pickup (used for UI) */
	bool bInteractionIsWeapon;
};
