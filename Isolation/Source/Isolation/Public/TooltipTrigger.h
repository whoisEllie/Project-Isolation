// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "TooltipTrigger.generated.h"

UENUM()
enum class ETextType : uint8
{
	Text,
	KeyInput
};

USTRUCT(BlueprintType)
struct FTextStruct
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Default")
	FString Text;

	UPROPERTY(EditInstanceOnly, Category = "Default")
	UInputAction* InputAction;

	UPROPERTY(EditInstanceOnly, Category = "Default")
	ETextType TextType;
};

UCLASS()
class ISOLATION_API ATooltipTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Sets default values for this actor's properties */
	ATooltipTrigger();

	/** Whether to draw debug messages to the screen or not */
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowDebug = false;

	/** the title of the tooltip to display */
	UPROPERTY(EditInstanceOnly, Category = "Variables")
	FText TooltipTitle;

	/** The data used to generate the description text, allowing the inclusion of input mapping displays */
	UPROPERTY(EditInstanceOnly, Category = "Variables")
	TArray<FTextStruct> TextData;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Resets the popup and allows it to be called again  */
	void ResetPopup() const;

	/** Bound to the box component's on overlap, implements the same signature */
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	/** Generates text with embedded input button images from TextData struct */
	void GenerateText();

	/** The time to wait before which to reactivate the tooltip after it has been called */
	UPROPERTY(EditInstanceOnly)
	float ReactivateDelay = 300.0f;

	/** The timer handle that keeps track of the reactivate delay */
	FTimerHandle DisplayDelayHandle;

	/** The final concatenation of the tooltip's description obtained from TextData */
	FText TooltipDescription;

	/** The box component used for collision detection */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	UBoxComponent* BoxComponent;

#if WITH_EDITOR

	/** An arrow component used to make it easier to select Tooltip Triggers in engine. Only visible in the editor */
	UPROPERTY()
	UArrowComponent* DisplayComponent;

#endif
};