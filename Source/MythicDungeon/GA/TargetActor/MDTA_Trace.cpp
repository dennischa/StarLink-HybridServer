// Fill out your copyright notice in the Description page of Project Settings.

#include "GA/TargetActor/MDTA_Trace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h"
#include "MythicDungeon.h"
#include "Abilities/GameplayAbility.h"
#include "Attribute/MDCharacterAttributeSet.h"

AMDTA_Trace::AMDTA_Trace()
{
	SetReplicates(false);
	SetReplicatingMovement(false);
}

void AMDTA_Trace::ConfirmTargetingAndContinue()
{
	if (SourceActor)
	{
		FGameplayAbilityTargetDataHandle DataHandle = PerformTrace();
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
}

void AMDTA_Trace::ConfigureTrace(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;

	// Add source actor to ignore list
	if (SourceActor)
	{
		IgnoreActors.AddUnique(SourceActor);
	}
}

FGameplayAbilityTargetDataHandle AMDTA_Trace::PerformTrace()
{
	// Base implementation - override in subclasses
	FGameplayAbilityTargetDataHandle Handle;
	return Handle;
}

FVector AMDTA_Trace::GetTraceStart() const
{
	return StartLocation.GetTargetingTransform().GetLocation();
}

FVector AMDTA_Trace::GetTraceEnd(const FVector& Direction) const
{
	auto* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);

	if (!ASC)
	{
		UE_LOG(LogMD, Warning, TEXT("ASC not found"));
		return  FVector::ZeroVector; 
	}

	auto* AttributeSet = ASC->GetSet<UMDCharacterAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogMD, Warning, TEXT("AttributeSet not found"));
		return  FVector::ZeroVector; 
	}
	
	return GetTraceStart() + (Direction * AttributeSet->GetAttackDistance());
}

FCollisionQueryParams AMDTA_Trace::ConfigureCollisionParams() const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AMDTA_Trace), bTraceComplex);
	Params.AddIgnoredActors(IgnoreActors);
	return Params;
}

void AMDTA_Trace::DrawDebugTraceResult(const FVector& Start, const FVector& End, bool bHit, const TArray<FHitResult>& HitResults, float SphereRadius) const
{
	if (!bShowDebug || !GetWorld())
	{
		return;
	}

	if (HitResults.Num() > 0)
	{
		for (const FHitResult& Hit : HitResults)
		{
			DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Red, false, DebugDrawTime, 0, 2.0f);
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 15.0f, FColor::Red, false, DebugDrawTime);

			if (SphereRadius > 0.0f)
			{
				DrawDebugSphere(GetWorld(), Hit.ImpactPoint, SphereRadius, 12, FColor::Red, false, DebugDrawTime);
			}
			else
			{
				DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.0f, 12, FColor::Red, false, DebugDrawTime);
			}
		}

		if (!HitResults.Last().bBlockingHit)
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, DebugDrawTime, 0, 1.0f);
		}
	}
	else
	{
		FColor LineColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), Start, End, LineColor, false, DebugDrawTime, 0, 1.0f);

		if (SphereRadius > 0.0f)
		{
			FColor SphereColor = bHit ? FColor::Red : FColor::Green;
			DrawDebugSphere(GetWorld(), Start, SphereRadius, 12, FColor::Green, false, DebugDrawTime);
			DrawDebugSphere(GetWorld(), End, SphereRadius, 12, SphereColor, false, DebugDrawTime);
		}
	}
}
