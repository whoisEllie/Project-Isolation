// Fill out your copyright notice in the Description page of Project Settings.


#include "STooltipPopup.h"
#include "SCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASTooltipPopup::ASTooltipPopup()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
}


// Called when the game starts or when spawned
void ASTooltipPopup::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ASTooltipPopup::OnOverlap);
}

// Generates text with embedded input button images from TextData struct
void ASTooltipPopup::GenerateText()
{
	FString Result = "";
	const UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

	TArray<FInputActionKeyMapping> OutMappings;
	
	for (FTextStruct TextStruct : TextData)
	{
		switch (TextStruct.TextType)
		{
			case ETextType::Text:
				Result.Append(TEXT("<Text>"));
				Result += TextStruct.Input;
				Result.Append(TEXT("</>"));
				break;

		case ETextType::KeyInput:
				OutMappings.Empty();
				Settings->GetInputSettings()->GetActionMappingByName(FName(*TextStruct.Input), OutMappings);
				for (FInputActionKeyMapping KeyMapping : OutMappings)
				{
					if (KeyConversionMap.Contains(KeyMapping.Key.ToString()))
					{
						Result.Append(TEXT("<img id=\""));
						Result += FString(KeyConversionMap[KeyMapping.Key.ToString()]);
						Result.Append(TEXT("\"/>"));
					}
					else
					{
						Result += KeyMapping.Key.ToString();
					}
				}
				break;
		}
	}

	TooltipDescription = FText::FromString(Result);
}


void ASTooltipPopup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	GenerateText();
	
	if (ShouldDisplay)
	{
		if (bShowDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Overlapped!"));
		}
		const ASCharacter* PlayerCharacter = Cast<ASCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			if (bShowDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Confirmed Playercharacter overlap"));
			}

			PlayerCharacter->UserWidget->ReceiveTooltipInput(TooltipTitle, TooltipDescription);
			ShouldDisplay = false;
			GetWorldTimerManager().SetTimer(DisplayDelayHandle, this, &ASTooltipPopup::ResetDisplay, 30.0f, false, 30.0f);
		}
	}
}

void ASTooltipPopup::ResetDisplay()
{
	ShouldDisplay = true;
}

