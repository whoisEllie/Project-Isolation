// Fill out your copyright notice in the Description page of Project Settings.


#include "CrystalElement.h"
#include "Components/SphereComponent.h"

// Sets default values
ACrystalElement::ACrystalElement()
{
	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(RootComponent);
	
	InfluenceRadiusSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Influence Sphere"));
	InfluenceRadiusSphere->SetSphereRadius(InfluenceSphereRadius);
	InfluenceRadiusSphere->SetupAttachment(CrystalMesh);
}

void ACrystalElement::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InfluenceRadiusSphere->SetSphereRadius(InfluenceSphereRadius);
}

void ACrystalElement::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Hello world!"));
}

// Called when the game starts or when spawned
void ACrystalElement::BeginPlay()
{
	Super::BeginPlay();

	InfluenceRadiusSphere->OnComponentBeginOverlap.AddDynamic(this, &ACrystalElement::HandleOverlap);
	
}
