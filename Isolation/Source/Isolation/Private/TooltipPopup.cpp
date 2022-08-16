// Fill out your copyright notice in the Description page of Project Settings.


#include "TooltipPopup.h"
#include "FPSCharacter.h"
#include "Components/BoxComponent.h"
#include "func_lib/FInputHelper.h"
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
				Result += TextStruct.Text;
				Result.Append(TEXT("</>"));
				break;

		case ETextType::KeyInput:

				const AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
				TArray<FEnhancedActionKeyMapping> Mappings = PlayerCharacter->GetBaseMappingContext()->GetMappings();

				FString InputDisplayText = "";
		
				for (FEnhancedActionKeyMapping Mapping : Mappings)
				{
					if (Mapping.Action == TextStruct.InputAction)
					{
						InputDisplayText.Append("<img id=\"" + FInputHelper::KeyConversionMap[Mapping.Key.GetDisplayName().ToString()] + "\"/> ");
					}
				}

				Result.Append(InputDisplayText);

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
		const AFPSCharacter* PlayerCharacter = Cast<AFPSCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			if (bShowDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Confirmed Playercharacter overlap"));
			}

			PlayerCharacter->GetPlayerHud()->ReceiveTooltipInput(TooltipTitle, TooltipDescription);
			ShouldDisplay = false;
			GetWorldTimerManager().SetTimer(DisplayDelayHandle, this, &ASTooltipPopup::ResetDisplay, 300.0f, false, 300.0f);
		}
	}
}

void ASTooltipPopup::ResetDisplay()
{
	ShouldDisplay = true;
}

