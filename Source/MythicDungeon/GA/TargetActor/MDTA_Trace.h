// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Physics/MDCollision.h"
#include "MDTA_Trace.generated.h"

/**
 * Base class for Trace-based TargetActors
 * - Designed for Object Pooling to avoid spawn costs
 * - Subclass to implement specific trace types (Line, Sphere, Box, etc.)
 */
UCLASS(Abstract)
class MYTHICDUNGEON_API AMDTA_Trace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMDTA_Trace();
	/** Initialize the trace with parameters before use */
	virtual void ConfigureTrace(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	virtual void ConfirmTargetingAndContinue() override;
protected:

	/** Perform the trace and return target data (implemented by subclasses) */
	virtual FGameplayAbilityTargetDataHandle PerformTrace();

	/** Collision channel to trace against */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = CCHANNEL_MDACTION;

	/** Whether to trace complex collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	bool bTraceComplex = false;

	/** Actors to ignore during trace */
	UPROPERTY(BlueprintReadWrite, Category = "Trace")
	TArray<AActor*> IgnoreActors;

	/** Whether to show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Debug")
	bool bShowDebug = false;

	/** Debug draw time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Debug", meta = (EditCondition = "bShowDebug"))
	float DebugDrawTime = 2.0f;

	/** Whether to return all hits or just the first one */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	bool bReturnMultipleHits = false;

	/** Maximum number of hits to return (if bReturnMultipleHits is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bReturnMultipleHits"))
	int32 MaxHits = 5;

	/** Draw debug visualization for hit results */
	void DrawDebugTraceResult(const FVector& Start, const FVector& End, bool bHit, const TArray<FHitResult>& HitResults, float SphereRadius = 0.0f) const;

	/** Helper to get trace start location */
	FVector GetTraceStart() const;

	/** Helper to get trace end location based on direction */
	FVector GetTraceEnd(const FVector& Direction) const;

	/** Helper to build query params */
	FCollisionQueryParams ConfigureCollisionParams() const;
};
