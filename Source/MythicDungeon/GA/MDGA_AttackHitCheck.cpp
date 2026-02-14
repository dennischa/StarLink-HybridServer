// Fill out your copyright notice in the Description page of Project Settings.


#include "GA/MDGA_AttackHitCheck.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MythicDungeon.h"
#include "AbilityTask/MDAT_Trace.h"
#include "TargetActor/MDTA_SphereTrace.h"

UMDGA_AttackHitCheck::UMDGA_AttackHitCheck()
{
	// Set ability to run on server
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	// Set instancing policy - instance per execution for clean state
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

	// Set replication policy
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
}

void UMDGA_AttackHitCheck::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	MD_GA_NETWORK_LOG(LogMDNetwork, Log, TEXT("Begin"));

	UMDAT_Trace* TraceAbilityTask = UMDAT_Trace::CreateTask(this, TargetActorClass);
	TraceAbilityTask->OnComplete.AddDynamic(this, &UMDGA_AttackHitCheck::OnTraceResultCallback);
	TraceAbilityTask->ReadyForActivation();
}

void UMDGA_AttackHitCheck::OnTraceResultCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	MD_GA_NETWORK_LOG(LogMDNetwork, Log, TEXT("Begin"));
	
	auto* SourceASC = GetAbilitySystemComponentFromActorInfo_Checked();

	if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetDataHandle, 0))
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(AttackDamageEffect);

		if (EffectSpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
		}
	}
}
 