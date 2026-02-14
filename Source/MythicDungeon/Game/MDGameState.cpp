// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MDGameState.h"

#include "StarLinkSubsystem.h"

void AMDGameState::BeginPlay()
{
	Super::BeginPlay();

	UStarLinkSubsystem::Get(GetWorld());
}
