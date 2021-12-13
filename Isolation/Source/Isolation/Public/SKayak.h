// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SInteractInterface.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/TimelineComponent.h"
#include "SKayak.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class UArrowComponent;

UCLASS()
class ISOLATION_API ASKayak : public APawn, public ISInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASKayak();

	virtual void Interact() override;

	void MoveForward(float value);

	void MoveRight(float value);

	void PossessPlayer();

	void TimelineProgress(float Value);

	FTimerHandle UpdateMovementTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MovementTickSpeed;

	void UpdateMovement();

	float UpdateSpeed(float Current, float Max, float IncreaseSpeed, float DecreaseSpeed, float Input, bool Changing);

	float MoveForwardValue;
	
	float MoveRightValue;

	// Determines the maximum forward speed of the kayak
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxForwardSpeed;

	// Determines the maximum sideways speed of the kayak
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxSidewaysSpeed;

	// Determines the maximum rotation speed of the Kayak
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxRotationSpeed;

	// Keeps track of the currently input forward speed
	float InputForwardSpeed;

	// Keeps track of the currently input sideways speed
	float InputSidewaysSpeed;

	// Keeps track of the currently input rotation
	float InputRotation;

	// Determines how fast the movement slows down
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SpeedSlowdown;

	// Determines how fast the rotation slows down
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RotationSlowdown;

	float ForwardMovement;

	float SidewaysMovement;

	float Rotation;

	UPROPERTY()
	APawn* PlayerPawn;

	UPROPERTY()
	AController* SavedController;

	bool bIsPossessed;
	
	FVector PlayerCameraLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ForwardSpeedMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SidewardSpeedMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RotationSpeedMultiplier;

	bool bIsMovingForward;

	bool bIsMovingRight;

	bool bIsRotating;
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UArrowComponent* PlayerSpawnComp;

	FTimeline CameraInterpTimeline;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
