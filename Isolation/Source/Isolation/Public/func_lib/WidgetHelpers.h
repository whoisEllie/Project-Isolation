// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CanvasPanel.h"

/**
 * 
 */
class ISOLATION_API WidgetHelpers
{
public:
	WidgetHelpers();
	~WidgetHelpers();
	
	static UUserWidget* GetParentCanvas(const UWidget* Widget);
};
