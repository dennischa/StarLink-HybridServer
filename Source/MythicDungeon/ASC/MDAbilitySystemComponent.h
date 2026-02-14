// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MDAbilitySystemComponent.generated.h"

/**
 * Custom Ability System Component for MythicDungeon
 * Handles ability replication and execution for multiplayer
 */
UCLASS()
class MYTHICDUNGEON_API UMDAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMDAbilitySystemComponent();

	// Initialize abilities and effects
	void InitializeAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& AbilitiesToGive);
	void InitializeEffects(const TArray<TSubclassOf<class UGameplayEffect>>& EffectsToApply);

	// Called when character dies
	virtual void OnCharacterDeath();

	// Server only
	bool bCharacterAbilitiesGiven = false;
};
