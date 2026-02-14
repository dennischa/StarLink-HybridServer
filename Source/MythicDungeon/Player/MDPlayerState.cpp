// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MDPlayerState.h"

#include "AbilitySystemComponent.h"
#include "ASC/MDAbilitySystemComponent.h"
#include "Attribute/MDCharacterAttributeSet.h"

AMDPlayerState::AMDPlayerState()
{
	ASC = CreateDefaultSubobject<UMDAbilitySystemComponent>(TEXT("ASC"));

	ASC->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UMDCharacterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AMDPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}
