// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MDAttackHitCheck.generated.h"

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API UAnimNotify_MDAttackHitCheck : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_MDAttackHitCheck();

protected:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:

	UPROPERTY(EditAnywhere, meta = (Categories = Event))
	FGameplayTag TriggerGameplayTag;
	
};
