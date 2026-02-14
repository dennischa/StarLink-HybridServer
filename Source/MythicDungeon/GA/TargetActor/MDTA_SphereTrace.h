// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA/TargetActor/MDTA_Trace.h"
#include "MDTA_SphereTrace.generated.h"

/**
 * Sphere Trace TargetActor
 * - Performs a sphere sweep trace from start to end
 * - Useful for melee attacks, projectile collision, etc.
 */
UCLASS()
class MYTHICDUNGEON_API AMDTA_SphereTrace : public AMDTA_Trace
{
	GENERATED_BODY()

public:
	AMDTA_SphereTrace();

	virtual FGameplayAbilityTargetDataHandle PerformTrace() override;
};
