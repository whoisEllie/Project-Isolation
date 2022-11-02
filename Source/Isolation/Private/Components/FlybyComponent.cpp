// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/FlybyComponent.h"


// Sets default values for this component's properties
UFlybyComponent::UFlybyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	FlybyAreaComponent = CreateDefaultSubobject<USphereComponent>(TEXT("FlybyAreaComponent"));
	if (GetOwner())
	{
		FlybyAreaComponent->SetupAttachment(GetOwner()->GetRootComponent());
	}
}


// Called when the game starts
void UFlybyComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFlybyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

