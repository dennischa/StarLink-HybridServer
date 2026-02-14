// Fill out your copyright notice in the Description page of Project Settings.

#include "GA/AbilityTask/MDAT_Trace.h"
#include "GA/TargetActor/MDTA_Trace.h"
#include "ASC/MDAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"

UMDAT_Trace::UMDAT_Trace()
{
}

UMDAT_Trace* UMDAT_Trace::CreateTask(UGameplayAbility* OwningAbility, TSubclassOf<AMDTA_Trace> TargetActorClass)
{
	UMDAT_Trace* NewTask = NewAbilityTask<UMDAT_Trace>(OwningAbility);
	NewTask->TargetActorClass = TargetActorClass;

	return NewTask;
}

void UMDAT_Trace::Activate()
{
	Super::Activate();

	SpawnAndInitializeTargetActor();

	if (SpawnedTargetActor)
	{
		// Perform trace immediately
		SpawnedTargetActor->ConfirmTargeting();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UMDAT_Trace: Failed to spawn TargetActor"));
		EndTask();
	}
}

void UMDAT_Trace::OnDestroy(bool bInOwnerFinished)
{
	// Return TargetActor to pool instead of destroying
	FinalizeTargetActor();

	Super::OnDestroy(bInOwnerFinished);
}

void UMDAT_Trace::SpawnAndInitializeTargetActor()
{
	if (!TargetActorClass)
	{
		return;
	}

	// Spawn TargetActor directly   
	SpawnedTargetActor = Cast<AMDTA_Trace>(GetWorld()->SpawnActorDeferred<AGameplayAbilityTargetActor>(TargetActorClass, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));

	if (SpawnedTargetActor)
	{
		SpawnedTargetActor->TargetDataReadyDelegate.AddUObject(this, &UMDAT_Trace::OnTargetDataReadyCallback);
		// Configure for this ability
		SpawnedTargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();
		SpawnedTargetActor->OwningAbility = Ability;
		SpawnedTargetActor->SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

		// Setup trace parameters
		FGameplayAbilityTargetingLocationInfo StartLoc;
		StartLoc.LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;
		StartLoc.SourceActor = SpawnedTargetActor->SourceActor;
		
		SpawnedTargetActor->ConfigureTrace(StartLoc);

		if (auto* ASC = AbilitySystemComponent.Get())
		{
			ASC->SpawnedTargetActors.Push(SpawnedTargetActor);
			SpawnedTargetActor->StartTargeting(Ability);
		}
	}
}

void UMDAT_Trace::FinalizeTargetActor()
{
	if (!SpawnedTargetActor)
	{
		return;
	}

	// Destroy TargetActor
	SpawnedTargetActor->Destroy();
	SpawnedTargetActor = nullptr;
}

void UMDAT_Trace::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& DataHandle)
{
	// Broadcast result
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnComplete.Broadcast(DataHandle);
	}

	// Clean up
	EndTask();
}
