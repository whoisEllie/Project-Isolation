// Fill out your copyright notice in the Description page of Project Settings.


#include "func_lib/WidgetHelpers.h"

WidgetHelpers::WidgetHelpers()
{
}

WidgetHelpers::~WidgetHelpers()
{
}

UUserWidget* WidgetHelpers::GetParentCanvas(const UWidget* Widget)
{
	UObject* Outer = Widget->GetOuter();
	
	while (!IsValid(Cast<UUserWidget>(Outer)))
	{
		Outer = Outer->GetOuter();
	}
	return Cast<UUserWidget>(Outer);
}
