// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacterController.h"

AFPSCharacterController::AFPSCharacterController()
{
	TeamId = FGenericTeamId(10);
}

FGenericTeamId AFPSCharacterController::GetGenericTeamId() const
{
	return TeamId;
}
