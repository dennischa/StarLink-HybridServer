// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Attribute/MDCharacterAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "MDPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API AMDPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMDPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UMDCharacterAttributeSet* GetCharacterAttributeSet() const { return AttributeSet; }

protected:
	UPROPERTY(EditAnywhere, Category=GAS)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(EditAnywhere, Category=GAS)
	TObjectPtr<class UMDCharacterAttributeSet> AttributeSet;
};
