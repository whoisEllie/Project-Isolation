// Fill out your copyright notice in the Description page of Project Settings.


#include "TooltipTrigger.h"

#include "FPSCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetManagementComponent.h"
#include "func_lib/FInputHelper.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATooltipTrigger::ATooltipTrigger()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);

	DisplayComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("DisplayComponent"));
	DisplayComponent->SetupAttachment(RootComponent);
}


// Called when the game starts or when spawned
void ATooltipTrigger::BeginPlay()
{
	Super::BeginPlay();

	// Binding to our box component's on overlap function
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ATooltipTrigger::OnOverlap);

	GenerateText();
}


void ATooltipTrigger::GenerateText()
{
	FString Result = "";
	TArray<FInputActionKeyMapping> OutMappings;
	
	for (FTextStruct TextStruct : TextData)
	{
		// Based on our text data array, we concatenate the final string
		switch (TextStruct.TextType)
		{
			case ETextType::Text:
				// Wrapping our text in the text tag
				Result.Append(TEXT("<Text>"));
				Result += TextStruct.Text;
				Result.Append(TEXT("</>"));
				break;

			case ETextType::KeyInput:

				// Iterating through all the action mappings and adding the rich text images for the selected input to the screen
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


void ATooltipTrigger::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
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

		// Displaying the tool tip in the player character on overlap
		if (const UWidgetManagementComponent* WidgetManagementComponent = PlayerCharacter->FindComponentByClass<UWidgetManagementComponent>())
		{
			if (WidgetManagementComponent->GetPlayerHud())
			{
				WidgetManagementComponent->GetPlayerHud()->DisplayToolTip(TooltipTitle, TooltipDescription);
			}
		}
		
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetWorldTimerManager().SetTimer(DisplayDelayHandle, this, &ATooltipTrigger::ResetPopup, ReactivateDelay, false, ReactivateDelay);
	}
}

void ATooltipTrigger::ResetPopup() const
{
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

