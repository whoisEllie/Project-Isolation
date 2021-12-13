// Fill out your copyright notice in the Description page of Project Settings.


#include "SKayak.h"
#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASKayak::ASKayak()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	PlayerSpawnComp = CreateDefaultSubobject<UArrowComponent>(TEXT("arrowComo"));
	PlayerSpawnComp->SetupAttachment(RootComponent);
}

void ASKayak::Interact()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!SavedController)
	{
		SavedController = PlayerPawn->GetController();
	}
	
	// Disable state/movement
	ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	ConsumeMovementInputVector();

	// Unpossess the current pawn
	SavedController->UnPossess();

	// Possess pawn
	SavedController->Possess(this);
	bIsPossessed = true;
	
	// Animate camera
	/*if (PlayerCharacter)
	{
		PlayerCameraLocation = PlayerCharacter->CameraComp->GetComponentLocation();
		CameraComp->SetWorldLocation(PlayerCameraLocation);
	}
	CameraInterpTimeline.Play();*/
}

void ASKayak::MoveForward(float value)
{
	if (value != 0)
	{
		bIsMovingForward = true;
	}
	else
	{
		bIsMovingForward = false;
	}
	MoveForwardValue = value;
}

void ASKayak::MoveRight(float value)
{
	if (value != 0)
	{
		if (bIsMovingForward)
		{
			bIsRotating = true;
			bIsMovingRight = false;
		}
		else
		{
			bIsMovingRight = true;
			bIsRotating = false;
		}
	}
	else
	{
		bIsMovingRight = false;
		bIsRotating = false;
	}
	MoveRightValue = value;
}

void ASKayak::PossessPlayer()
{
	APlayerController* Controller = Cast<APlayerController>(GetController());
	Controller->UnPossess();
	PlayerPawn->SetActorLocation(PlayerSpawnComp->GetComponentLocation());
	PlayerPawn->SetActorRotation(PlayerSpawnComp->GetComponentRotation());
	Controller->Possess(PlayerPawn);
	bIsPossessed = false;
}

void ASKayak::TimelineProgress(float value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, value)
	/*FVector NewLocation = FMath::Lerp(PlayerCameraLocation, CameraComp->GetComponentLocation(), value);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, NewLocation.ToString());
	CameraComp->SetWorldLocation(NewLocation);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Timeline tick"));*/
}

void ASKayak::UpdateMovement()
{
	ForwardMovement = UpdateSpeed(ForwardMovement, MaxForwardSpeed, ForwardSpeedMultiplier, SpeedSlowdown, MoveForwardValue, bIsMovingForward);
	SidewaysMovement = UpdateSpeed(SidewaysMovement, MaxSidewaysSpeed, SidewardSpeedMultiplier, SpeedSlowdown, MoveRightValue, bIsMovingRight);
	Rotation = UpdateSpeed(Rotation, MaxRotationSpeed, RotationSpeedMultiplier, RotationSlowdown, MoveRightValue, bIsRotating);

	FVector FinalMovement = FVector::ZeroVector;
	FinalMovement = ((GetActorForwardVector() * ForwardMovement) + (GetActorRightVector() * SidewaysMovement));
	FRotator FinalRotator = FRotator::ZeroRotator;
	FinalRotator.Yaw = Rotation;
	SetActorLocation(GetActorLocation() + FinalMovement);
	SetActorRotation(GetActorRotation() + FinalRotator);
}

float ASKayak::UpdateSpeed(float Current, float Max, float IncreaseSpeed, float DecreaseSpeed, float Input, bool Changing)
{
	if(Changing)
	{
		if (Input > 0)
		{
			if (Current < (Max - IncreaseSpeed))
			{
				Current += IncreaseSpeed;
			}
			else
			{
				Current = Max;
			}
		}
		else
		{
			if (Current > (-Max + IncreaseSpeed))
			{
				Current -= IncreaseSpeed;
			}
			else
			{
				Current = -Max;
			}
		}
	}
	else
	{
		if (Current > DecreaseSpeed)
		{
			Current -= DecreaseSpeed;
		}
		else if (Current < -DecreaseSpeed)
		{
			Current += DecreaseSpeed;
		}
		else
		{
			Current = 0;
		}
	}
	return Current;
}


// Called when the game starts or when spawned
void ASKayak::BeginPlay()
{
	Super::BeginPlay();

	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
		CameraInterpTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CameraInterpTimeline.SetLooping(false);
	}
	
	GetWorldTimerManager().SetTimer(UpdateMovementTimer, this, &ASKayak::UpdateMovement, MovementTickSpeed, true, MovementTickSpeed);
}

// Called every frame
void ASKayak::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void ASKayak::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASKayak::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASKayak::MoveRight);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASKayak::PossessPlayer);
}

