// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_MDAttackHitCheck.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

UAnimNotify_MDAttackHitCheck::UAnimNotify_MDAttackHitCheck()
{
}

FString UAnimNotify_MDAttackHitCheck::GetNotifyName_Implementation() const
{
	return TEXT("MDAttackHitCheck");
}

void UAnimNotify_MDAttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		AActor* OwnerActor = MeshComp->GetOwner();
		if (OwnerActor)
		{
			FGameplayEventData PayloadData;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, TriggerGameplayTag, PayloadData);
		}
	}
}
