// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MDAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class MYTHICDUNGEON_API UMDAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class ACharacter> Owner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UCharacterMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	bool bIsInAir = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	bool bIsAccelerating = false;
};
