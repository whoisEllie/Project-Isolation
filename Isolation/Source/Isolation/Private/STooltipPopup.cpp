// Fill out your copyright notice in the Description page of Project Settings.


#include "STooltipPopup.h"
#include "SCharacter.h"
#include "Components/BoxComponent.h"
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

void ASTooltipPopup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
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

			PlayerCharacter->UserWidget->ReceiveInput(TooltipTitle, TooltipDescription);
			ShouldDisplay = false;
			GetWorldTimerManager().SetTimer(DisplayDelayHandle, this, &ASTooltipPopup::ResetDisplay, 30.0f, false, 30.0f);
		}
	}
}

void ASTooltipPopup::ResetDisplay()
{
	ShouldDisplay = true;
}

