// Fill out your copyright notice in the Description page of Project Settings.


#include "CrystalElement.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ACrystalElement::ACrystalElement()
{
	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(RootComponent);
	
	InfluenceRadiusSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Influence Sphere"));
	InfluenceRadiusSphere->SetSphereRadius(InfluenceSphereRadius);
	InfluenceRadiusSphere->SetupAttachment(CrystalMesh);

	SelectionBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard Component"));
	SelectionBillboard->SetupAttachment(CrystalMesh);
}

void ACrystalElement::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InfluenceRadiusSphere->SetSphereRadius(InfluenceSphereRadius);
}

void ACrystalElement::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Entered sphere of influence"));

	if ((FPSCharacterReference = Cast<AFPSCharacter>(OtherActor)))
	{
		GetWorldTimerManager().SetTimer(BreathDamageTimerHandle, this, &ACrystalElement::ApplyBreathDamage, TimeFrame, true, TimeFrame);
	}
}

void ACrystalElement::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Exited sphere of influence"));

	GetWorldTimerManager().ClearTimer(BreathDamageTimerHandle);

	if (FPSCharacterReference)
	{
		FPSCharacterReference->ResumeBreathHealingTimer();
	}
}

void ACrystalElement::ApplyBreathDamage() const
{
	if (FPSCharacterReference)
	{
		FPSCharacterReference->SubtractXFromBreathHealth(MaxBreathDamage * ((FMath::Clamp((1 - FVector::Distance(GetActorLocation(), FPSCharacterReference->GetActorLocation())/InfluenceSphereRadius), 0.25f, 1.0f))));
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Subtracted from breath health"));
}

// Called when the game starts or when spawned
void ACrystalElement::BeginPlay()
{
	Super::BeginPlay();

	InfluenceRadiusSphere->OnComponentEndOverlap.AddDynamic(this, &ACrystalElement::EndOverlap);
	InfluenceRadiusSphere->OnComponentBeginOverlap.AddDynamic(this, &ACrystalElement::BeginOverlap);
}
