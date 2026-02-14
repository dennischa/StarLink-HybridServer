// Fill out your copyright notice in the Description page of Project Settings.

#include "GA/TargetActor/MDTA_SphereTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "DrawDebugHelpers.h"
#include "MythicDungeon.h"
#include "Attribute/MDCharacterAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

AMDTA_SphereTrace::AMDTA_SphereTrace()
{
}

FGameplayAbilityTargetDataHandle AMDTA_SphereTrace::PerformTrace()
{
	ACharacter* Character = CastChecked<ACharacter>(SourceActor);
	FGameplayAbilityTargetDataHandle Handle;

	if (!GetWorld())
	{
		return Handle;
	}
	
	auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);

	if (!ASC)
	{
		UE_LOG(LogMD, Warning, TEXT("ASC not found"));
		return Handle;
	}

	auto* AttributeSet = ASC->GetSet<UMDCharacterAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogMD, Warning, TEXT("AttributeSet not found"));
		return  Handle; 
	}

	const float AttackDistance = AttributeSet->GetAttackDistance();
	const float AttackRadius = AttributeSet->GetAttackRadius();

	// Get trace start/end
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector Start = Character->GetActorLocation() + ForwardVector * Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector End = GetTraceEnd(ForwardVector);
	
	const float SphereRadius = AttributeSet->GetAttackRadius();

	// Configure collision
	FCollisionQueryParams Params = ConfigureCollisionParams();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

	if (bReturnMultipleHits)
	{
		// Multi-sphere trace
		TArray<FHitResult> HitResults;
		GetWorld()->SweepMultiByChannel(
			HitResults,
			Start,
			End,
			FQuat::Identity,
			TraceChannel,
			SphereShape,
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

		if (bShowDebug)
		{
			FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
			float CapsuleHalfHeight = AttackDistance * 0.5f;
			DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(ForwardVector).ToQuat(), FColor::Green, false, DebugDrawTime);
		}
		DrawDebugTraceResult(Start, End, HitResults.Num() > 0, HitResults, SphereRadius);
	}
	else
	{
		// Single sphere trace
		FHitResult HitResult;
		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult,
			Start,
			End,
			FQuat::Identity,
			TraceChannel,
			SphereShape,
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
		DrawDebugTraceResult(Start, End, bHit, SingleResult, SphereRadius);
	}

	return Handle;
}
