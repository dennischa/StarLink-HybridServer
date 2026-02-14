// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDGA_BasicAttack.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemInterface.h"
#include "MythicDungeon.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/MDCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UMDGA_BasicAttack::UMDGA_BasicAttack()
{
	// Set ability to run with client prediction for responsive gameplay
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Set instancing policy - instance per execution for clean state
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UMDGA_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	MD_GA_NETWORK_LOG(LogMDNetwork, Log, TEXT("Begin"));
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Play animation montage if available
	AMDCharacter* Character = Cast<AMDCharacter>(ActorInfo->AvatarActor.Get());
	
	if (Character)
	{
		if (Character->GetController())
		{
			Character->GetController()->StopMovement();
		}
		Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
		
		UAnimMontage* AttackMontage = Character->GetAttackMontage();
		auto PlayAttackMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayAttack"), AttackMontage, 1.0f, TEXT("Attack1"));

		PlayAttackMontageTask->OnCompleted.AddDynamic(this, &UMDGA_BasicAttack::OnCompletedCallback);
		PlayAttackMontageTask->OnInterrupted.AddDynamic(this, &UMDGA_BasicAttack::OnInterruptedCallback);

		PlayAttackMontageTask->ReadyForActivation();

		// 혹시 몰라서 동기화 빠르게 해보려고
		if (HasAuthority(&ActivationInfo))
		{
			Character->ForceNetUpdate();
		}
	}

	MD_GA_NETWORK_LOG(LogMDNetwork, Log, TEXT("End"));
}

void UMDGA_BasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	MD_GA_NETWORK_LOG(LogMDNetwork, Log, TEXT("Begin"));
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	auto* Character = Cast<AMDCharacter>(ActorInfo->AvatarActor.Get());
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	MD_GA_NETWORK_LOG(LogMDNetwork, Log, TEXT("End"));
}

void UMDGA_BasicAttack::OnCompletedCallback()
{
	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMDGA_BasicAttack::OnInterruptedCallback()
{
	bool bReplicateEndAbility = true;
	bool bWasCancelled = true;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}
