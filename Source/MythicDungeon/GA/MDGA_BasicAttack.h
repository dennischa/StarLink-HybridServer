// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MDGA_BasicAttack.generated.h"

/**
 * Non-targeted basic attack ability for top-down RPG
 * Attacks in the direction the character is facing
 * Uses sphere trace to detect enemies in front of the character
 */
UCLASS()
class MYTHICDUNGEON_API UMDGA_BasicAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMDGA_BasicAttack();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// Damage effect to apply
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	UFUNCTION()
	void OnCompletedCallback();

	UFUNCTION()
	void OnInterruptedCallback();

private:
	FTimerHandle AttackTimerHandle;
};
