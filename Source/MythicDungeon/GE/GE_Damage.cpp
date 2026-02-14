// Copyright Epic Games, Inc. All Rights Reserved.

#include "GE_Damage.h"
#include "Attribute/MDCharacterAttributeSet.h"

UGE_Damage::UGE_Damage()
{
	// Set duration policy to instant (damage is applied immediately)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Configure a modifier that adds to the Damage attribute
	// The actual damage value should be set in the Blueprint child class
	FGameplayModifierInfo DamageModifier;
	DamageModifier.ModifierMagnitude = FScalableFloat(10.f); // Default value, override in BP
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.Attribute = UMDCharacterAttributeSet::GetDamageAttribute();

	Modifiers.Add(DamageModifier);
}
