// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "TooltipPopup.generated.h"

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
	FString Input;

	UPROPERTY(EditInstanceOnly, Category = "Default")
	ETextType TextType;
};

UCLASS()
class ISOLATION_API ASTooltipPopup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASTooltipPopup();

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	UBoxComponent* BoxComponent;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);
	
	void GenerateText();
	
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowDebug = false;

	UPROPERTY(EditInstanceOnly, Category = "Variables")
	FText TooltipTitle;
	
	FText TooltipDescription;

	UPROPERTY(EditInstanceOnly, Category = "Variables")
	TArray<FTextStruct> TextData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ResetDisplay();

	FTimerHandle DisplayDelayHandle;

	bool ShouldDisplay = true;

	// Converts UE input names to Decorator key names
	UPROPERTY(EditDefaultsOnly, Category = "KeyMap")
	TMap<FString, FString> KeyConversionMap;

};