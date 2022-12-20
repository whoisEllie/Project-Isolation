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
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
		}
	}
	return ETeamAttitude::Neutral;
}

void AAICharacterController::PickTargetActor(TArray<AActor*> InArray)
{
	TargetsArray = InArray;

	QuickSort(&TargetsArray, 0, TargetsArray.Num() - 1);

	TargetActor = TargetsArray[0];
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TargetActor->GetName());
}

void AAICharacterController::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, TEXT("Binding perception update"));
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
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Perception Updated"));
	TargetsArray = UpdatedActors;
	QuickSort(&TargetsArray, 0, TargetsArray.Num() - 1);
	TargetActor = TargetsArray[0];
}
