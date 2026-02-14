// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/TargetActor/MDTA_Trace.h"
#include "MDTA_LineTrace.generated.h"

/**
 * Line Trace TargetActor
 * - Performs a simple line trace from start to end
 * - Supports single or multi-hit traces
 */
UCLASS()
class MYTHICDUNGEON_API AMDTA_LineTrace : public AMDTA_Trace
{
	GENERATED_BODY()

public:
	AMDTA_LineTrace();

	virtual FGameplayAbilityTargetDataHandle PerformTrace() override;
};
