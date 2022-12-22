// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacterController.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

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
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
		}
	}
	return ETeamAttitude::Neutral;
}

void AAICharacterController::BeginPlay()
{
	Super::BeginPlay();

	AiPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AAICharacterController::HandlePerceptionUpdate);
}

int AAICharacterController::Partition(TArray<AActor*> *InArray, int Start, int End) const
{
	const float Pivot = (*InArray)[Start]->GetDistanceTo(this);

	int Count = 0;
	for (int i = Start + 1; i <= End; ++i)
	{
		if ((*InArray)[i]->GetDistanceTo(this) <= Pivot)
		{
			Count++;	
		}	
	}

	int PivotIndex = Start + Count;
	Swap(InArray[PivotIndex], InArray[Start]);

	int i = Start, j = End;

	while (i < PivotIndex && j > PivotIndex)
	{
		while ((*InArray)[i]->GetDistanceTo(this) <= Pivot)
		{
			i++;
		}

		while ((*InArray)[j]->GetDistanceTo(this) > Pivot)
		{
			j--;
		}

		if (i < PivotIndex && j > PivotIndex)
		{
			Swap(InArray[i++], InArray[j--]);
		}
	}

	return PivotIndex;
}

void AAICharacterController::QuickSort(TArray<AActor*> *InArray, int Start, int End)
{
	if (Start >= End) return;

	const int P = Partition(InArray, Start, End);

	QuickSort(InArray, Start, P-1);

	QuickSort(InArray, P+1, End);
}



void AAICharacterController::HandlePerceptionUpdate(const TArray<AActor*>& UpdatedActors)
{
	UpdateTargetActor();	
}

void AAICharacterController::UpdateTargetActor()
{
	AiPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), TargetsArray);
	if (TargetsArray.Num() > 0)
	{
		TargetActor = TargetsArray[0];
	}
	else
	{
		TargetActor = nullptr;
	}
}
