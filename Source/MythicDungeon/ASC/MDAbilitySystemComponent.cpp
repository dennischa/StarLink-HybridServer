// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDAbilitySystemComponent.h"
#include "GameplayEffect.h"

UMDAbilitySystemComponent::UMDAbilitySystemComponent()
{
	// Enable replication
	SetIsReplicatedByDefault(true);

	// Set replication mode for multiplayer
	// Mixed mode: GameplayEffects are replicated to all, GameplayCues and Attributes are replicated to owner only
	ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

void UMDAbilitySystemComponent::InitializeAbilities(const TArray<TSubclassOf<UGameplayAbility>>& AbilitiesToGive)
{
	// Only execute on server
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitiesToGive)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, GetOwner());
			GiveAbility(AbilitySpec);
		}
	}

	bCharacterAbilitiesGiven = true;
}

void UMDAbilitySystemComponent::InitializeEffects(const TArray<TSubclassOf<UGameplayEffect>>& EffectsToApply)
{
	// Only execute on server
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(GetOwner());

	for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectsToApply)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle EffectSpec = MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
			if (EffectSpec.IsValid())
			{
				ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
			}
		}
	}
}

void UMDAbilitySystemComponent::OnCharacterDeath()
{
	// Cancel all active abilities
	CancelAllAbilities();

	// Add death tags or apply death effects here if needed
}
