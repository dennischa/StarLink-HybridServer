// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "MDCharacter.generated.h"

class UMDAbilitySystemComponent;
class UMDCharacterAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

/**
 * Base Character class with GAS support for MythicDungeon
 * Implements AbilitySystemInterface for multiplayer GAS functionality
 */
UCLASS()
class MYTHICDUNGEON_API AMDCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMDCharacter();

	// IAbilitySystemInterface implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Attribute accessors
	UFUNCTION(BlueprintCallable, Category = "MythicDungeon|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "MythicDungeon|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "MythicDungeon|Attributes")
	float GetAttackDamage() const;

	// Input binding for abilities
	UFUNCTION(BlueprintCallable, Category = "MythicDungeon|Abilities")
	void ActivateBasicAttack();

	UAnimMontage* GetAttackMontage() const { return AttackMontage;}

protected:
	virtual void OnRep_PlayerState() override;
	
	// Initialize character with abilities and effects
	virtual void InitializeAbilities();
	
	// Called on both server and client to initialize ASC
	virtual void InitializeAbilitySystemComponent();

	bool bAbilitySystemInitialized = false;
	
protected:
	// GAS Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MD|Abilities")
	TObjectPtr<UMDAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MD|Attributes")
	TObjectPtr<UMDCharacterAttributeSet> AttributeSet;

	// Default abilities to grant
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MD|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// Default effects to apply (for initial stats)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MD|GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	// Basic Attack Ability Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MD|GAS")
	TSubclassOf<UGameplayAbility> BasicAttackAbilityClass;

	// Tag for basic attack ability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MD|Abilities")
	FGameplayTag BasicAttackTag;

	// Animation montage to play
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|GAS")
	TObjectPtr<UAnimMontage> AttackMontage;
};
