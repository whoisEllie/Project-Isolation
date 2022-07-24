// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UHealthComponent*, HealthComponent, float,
                                             Health, float, HealthDelta, const class UDamageType*, DamageType,
                                             class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Isolation), meta=(BlueprintSpawnableComponent) )
class ISOLATION_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	float GetHealth() const { return Health; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Health Component")
	float DefaultHealth = 100.0f;
	
	float Health = 100.0f;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	                         AController* InstigatedBy, AActor* DamageCauser);
};
