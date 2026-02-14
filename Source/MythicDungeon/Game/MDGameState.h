// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MDGameState.generated.h"

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API AMDGameState : public AGameState
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
