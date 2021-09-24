// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;


UCLASS()
class ISOLATION_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

    //Hands mesh, assignable through blueprints
    UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComp;
	
	//Camera Comp - component for the FPS camera
	UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	//Spring Arm Comp - component for the spring arm, which is required to enable 'use control rotation'
	UPROPERTY(VisibleAnywhere, SaveGame, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;
    
    
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
    
	void MoveRight(float value);
    
	void LookUp(float value);
    
	void LookRight(float value);
	
	void ExecCrouch();
    
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
