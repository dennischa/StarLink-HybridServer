// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDCharacterAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UMDCharacterAttributeSet::UMDCharacterAttributeSet()
	: Health(100.f)
	, MaxHealth(100.f)
	, AttackDamage(10.f)
	, AttackRadius(50.f)
	, AttackDistance(50.f)
	, MovementSpeed(600.f)
	, Damage(0.f)
{
}

void UMDCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp health between 0 and MaxHealth
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Don't allow MaxHealth to go negative
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

void UMDCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Handle Damage meta attribute
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Get the damage value
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f); // Reset damage meta attribute

		if (LocalDamageDone > 0.f)
		{
			// Apply damage to health
			const float NewHealth = GetHealth() - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}
	}
	// Clamp Health
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UMDCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMDCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDCharacterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDCharacterAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDCharacterAttributeSet, AttackRadius, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDCharacterAttributeSet, AttackDistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMDCharacterAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void UMDCharacterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDCharacterAttributeSet, Health, OldHealth);
}

void UMDCharacterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDCharacterAttributeSet, MaxHealth, OldMaxHealth);
}

void UMDCharacterAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDCharacterAttributeSet, AttackDamage, OldAttackDamage);
}

void UMDCharacterAttributeSet::OnRep_AttackRadius(const FGameplayAttributeData& OldAttackRadius)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDCharacterAttributeSet, AttackRadius, OldAttackRadius);
}

void UMDCharacterAttributeSet::OnRep_AttackDistance(const FGameplayAttributeData& OldAttackDistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDCharacterAttributeSet, AttackDistance, OldAttackDistance);
}

void UMDCharacterAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMDCharacterAttributeSet, MovementSpeed, OldMovementSpeed);
}
