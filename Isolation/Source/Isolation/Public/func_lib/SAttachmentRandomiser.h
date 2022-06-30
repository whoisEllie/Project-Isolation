// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeaponBase.h"
#include "GameFramework/Actor.h"
#include "SAttachmentRandomiser.generated.h"

UCLASS()
class ISOLATION_API ASAttachmentRandomiser : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	
	TArray<FName> RandomiseAllAttachments(UDataTable* AttachmentDataTable);

	UFUNCTION(BlueprintCallable)
	TArray<FName> ReplaceIncompatibleAttachments(UDataTable* AttachmentDataTable, TArray<FName> CurrentAttachments);

	static TArray<FString> GetDataTableKeyColumnAsString(UDataTable* DataTable);
};
