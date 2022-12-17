// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacterController.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"

AAICharacterController::AAICharacterController(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	AiPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AiPerceptionComponent"));
	AAIController::SetGenericTeamId(FGenericTeamId(5));
}

ETeamAttitude::Type AAICharacterController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("WEEEE"));
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Cast Failed!"));
	return ETeamAttitude::Neutral;
}
