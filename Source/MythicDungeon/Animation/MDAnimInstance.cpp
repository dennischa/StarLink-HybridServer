// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/MDAnimInstance.h"
#include "MythicDungeon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetMathLibrary.h"

void UMDAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UMDAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Owner  || !Movement)
	{
		return;
	}

	Speed = Owner->GetVelocity().SizeSquared();
	bIsInAir = FMath::Abs(Movement->Velocity.Z) > KINDA_SMALL_NUMBER;
	bIsAccelerating = Speed > KINDA_SMALL_NUMBER;
}


