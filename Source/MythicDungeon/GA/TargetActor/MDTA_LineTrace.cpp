// Fill out your copyright notice in the Description page of Project Settings.

#include "GA/TargetActor/MDTA_LineTrace.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

AMDTA_LineTrace::AMDTA_LineTrace()
{
}

FGameplayAbilityTargetDataHandle AMDTA_LineTrace::PerformTrace()
{
	FGameplayAbilityTargetDataHandle Handle;

	if (!GetWorld())
	{
		return Handle;
	}

	// Get trace start/end
	FVector Start = GetTraceStart();
	FVector ForwardVector = StartLocation.GetTargetingTransform().GetRotation().GetForwardVector();
	FVector End = GetTraceEnd(ForwardVector);

	// Configure collision
	FCollisionQueryParams Params = ConfigureCollisionParams();

	if (bReturnMultipleHits)
	{
		// Multi-line trace
		TArray<FHitResult> HitResults;
		GetWorld()->LineTraceMultiByChannel(
			HitResults,
			Start,
			End,
			TraceChannel,
			Params
		);

		// Limit hits if needed
		if (MaxHits > 0 && HitResults.Num() > MaxHits)
		{
			HitResults.SetNum(MaxHits);
		}

		// Create target data for each hit
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.bBlockingHit)
			{
				FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
				Handle.Add(TargetData);
			}
		}

		DrawDebugTraceResult(Start, End, HitResults.Num() > 0, HitResults);
	}
	else
	{
		// Single line trace
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			TraceChannel,
			Params
		);

		if (bHit && HitResult.bBlockingHit)
		{
			FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			Handle.Add(TargetData);
		}

		TArray<FHitResult> SingleResult;
		if (bHit)
		{
			SingleResult.Add(HitResult);
		}
		DrawDebugTraceResult(Start, End, bHit, SingleResult);
	}

	return Handle;
}
