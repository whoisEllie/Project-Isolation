// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"

/** Class for modifying arrays of weapon attachments */
class ISOLATION_API FAttachmentHelpers
{
public:
	/**
	 * Obtain a random set of attachments (one of each type)
	 * @param AttachmentDataTable Data Table from which we pull attachments
	 * @warning Make sure that AttachmentDataTable is of type FAttachmentData
	 * @return A randomised array of weapon attachments
	 */
	static TArray<FName> RandomiseAllAttachments(UDataTable* AttachmentDataTable);

	/**
	 * Clean up attachment incompatibilities in the given attachment set
	 * @param AttachmentDataTable Data Table from which we pull attachments
	 * @param CurrentAttachments The array of the weapon's current attachments
	 * @warning Make sure that AttachmentDataTable is of type FAttachmentData
	 * @return A cleaned up array of weapon attachments with no incompatibilities
	 */
	static TArray<FName> ReplaceIncompatibleAttachments(UDataTable* AttachmentDataTable, TArray<FName> CurrentAttachments);

	/**
	 * Collect all keys from a data table
	 * @param DataTable The Data Table from which to collect keys
	 * @return An array of keys as strings
	 */
	static TArray<FString> GetDataTableKeyColumnAsString(UDataTable* DataTable);
};
