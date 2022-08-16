// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractionComponent.h"
#include "AmmoPickup.h"
#include "EnhancedInputComponent.h"
#include "FPSCharacter.h"
#include "InteractionActor.h"
#include "WeaponPickup.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

    CameraComponent = GetOwner()->FindComponentByClass<UCameraComponent>();
}

void UInteractionComponent::WorldInteract()
{
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("LineTrace"));
    
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;

    const AActor* Owner = GetOwner();
    
    if (Owner)
    {
        TraceParams.AddIgnoredActor(Owner);
    }

    FVector CameraLocation = FVector::ZeroVector;
    FRotator CameraRotation = FRotator::ZeroRotator;
    
    if (CameraComponent)
    {
        CameraLocation = CameraComponent->GetComponentLocation();
        CameraRotation = CameraComponent->GetComponentRotation(); 
    }
    
    const FVector TraceDirection = CameraRotation.Vector();
    const FVector TraceEndLocation = CameraLocation + TraceDirection * InteractDistance;

    if (GetWorld()->LineTraceSingleByChannel(InteractionHit, CameraLocation, TraceEndLocation, ECC_WorldStatic, TraceParams))
    {
        if(InteractionHit.GetActor()->GetClass()->ImplementsInterface(USInteractInterface::StaticClass()))
        {
            Cast<ISInteractInterface>(InteractionHit.GetActor())->Interact();
        }
    }
}

// Performing logic for the interaction indicator at the center of the screen
void UInteractionComponent::InteractionIndicator()
{
    bCanInteract = false;
    
    FCollisionQueryParams TraceParams;
    TraceParams.bTraceComplex = true;

    const AActor* Owner = GetOwner();
    
    if (Owner)
    {
        TraceParams.AddIgnoredActor(Owner);
    }
    
    FVector CameraLocation = FVector::ZeroVector;
    FRotator CameraRotation = FRotator::ZeroRotator;
    
    if (CameraComponent)
    {
        CameraLocation = CameraComponent->GetComponentLocation();
        CameraRotation = CameraComponent->GetComponentRotation(); 
    }
    
    const FVector TraceDirection = CameraRotation.Vector();
    const FVector TraceEndLocation = CameraLocation + TraceDirection * InteractDistance;

    if (GetWorld()->LineTraceSingleByChannel(InteractionHit, CameraLocation, TraceEndLocation, ECC_WorldStatic, TraceParams))
    {        
        if(InteractionHit.GetActor()->GetClass()->ImplementsInterface(USInteractInterface::StaticClass()))
        {
            bCanInteract = true;
            const ASInteractionActor* InteractionActor = Cast<ASInteractionActor>(InteractionHit.GetActor());
            AAmmoPickup* AmmoPickup = Cast<AAmmoPickup>(InteractionHit.GetActor());
            if (InteractionActor)
            {
                InteractText = InteractionActor->PopupDescription;
            }
            else if (AmmoPickup)
            {
                InteractText = AmmoPickup->GetPickupName();
            }
            else
            {
                InteractText = FText::GetEmpty();
            }
            
            const ASWeaponPickup* InteractedPickup = Cast<ASWeaponPickup>(InteractionHit.GetActor());
            if (InteractedPickup)
            {
                bInteractionIsWeapon = true;
                InteractText = InteractedPickup->WeaponName;
            }
            else
            {
                bInteractionIsWeapon = false;
            }
        }
    }
}

// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Checks to see if we are facing something to interact with, and updates the interaction indicator accordingly
    InteractionIndicator();
}

void UInteractionComponent::SetupInputComponent(UEnhancedInputComponent* PlayerInputComponent)
{
    if (WorldInteractAction)
    {            
        // Interacting with the world
        PlayerInputComponent->BindAction(WorldInteractAction, ETriggerEvent::Started, this, &UInteractionComponent::WorldInteract);
    }
}

