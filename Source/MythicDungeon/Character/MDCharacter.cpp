// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDCharacter.h"

#include "MythicDungeon.h"
#include "ASC/MDAbilitySystemComponent.h"
#include "Attribute/MDCharacterAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/MDPlayerState.h"

AMDCharacter::AMDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 상속 클래스에서 생성 필요
	ASC = nullptr;
	AttributeSet = nullptr;

	bReplicates = true;

	// Configure character movement for top-down
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Don't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
}

void AMDCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Client: Initialize ASC
	if (!bAbilitySystemInitialized)
	{
		InitializeAbilitySystemComponent();
	}
}

void AMDCharacter::InitializeAbilitySystemComponent()
{
	if (!ASC || bAbilitySystemInitialized)
	{
		return;
	}

	// Initialize the ASC with owner and avatar
	ASC->InitAbilityActorInfo(this, this);

	bAbilitySystemInitialized = true;
}

void AMDCharacter::InitializeAbilities()
{
	// Only run on server
	if (!HasAuthority() || !ASC)
	{
		return;
	}

	MD_NETWORK_LOG(LogMDNetwork, Log, TEXT("Begin"));

	// Grant default abilities
	if (DefaultAbilities.Num() > 0)
	{
		ASC->InitializeAbilities(DefaultAbilities);
	}

	// Add basic attack if specified
	if (BasicAttackAbilityClass)
	{
		FGameplayAbilitySpec AttackSpec(BasicAttackAbilityClass);
		AttackSpec.InputID = 0;
		
		ASC->GiveAbility(AttackSpec);
	}

	// Apply default effects (initial stats)
	if (DefaultEffects.Num() > 0)
	{
		ASC->InitializeEffects(DefaultEffects);
	}
}

UAbilitySystemComponent* AMDCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

float AMDCharacter::GetHealth() const
{
	if (!AttributeSet)
	{
		return 0.f;
	}

	return AttributeSet->GetHealth();
}

float AMDCharacter::GetMaxHealth() const
{
	if (!AttributeSet)
	{
		return 0.f;
	}

	return AttributeSet->GetMaxHealth();
}

float AMDCharacter::GetAttackDamage() const
{
	if (!AttributeSet)
	{
		return 0.f;
	}

	return AttributeSet->GetAttackDamage();
}

void AMDCharacter::ActivateBasicAttack()
{
	if (!ASC)
	{
		return;
	}

	// Try to activate ability by class
	if (BasicAttackAbilityClass)
	{
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(BasicAttackAbilityClass);
		if (Spec)
		{
			ASC->TryActivateAbility(Spec->Handle);
		}
	}
}
