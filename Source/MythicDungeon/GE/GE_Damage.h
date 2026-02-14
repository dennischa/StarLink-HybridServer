// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage.generated.h"

/**
 * Base Damage Gameplay Effect
 * This is a C++ base class that can be inherited in Blueprint
 * Configure the actual damage values in the Blueprint child class
 */
UCLASS()
class MYTHICDUNGEON_API UGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Damage();
};
