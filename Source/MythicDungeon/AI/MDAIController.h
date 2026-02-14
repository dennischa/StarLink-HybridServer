// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MDAIController.generated.h"

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API AMDAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMDAIController();

	void RunAI();

	void StopAI();

protected:
	virtual void OnPossess(APawn* InPawn) override;
private:
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<class UBlackboardData> BBAsset;
	
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<class UBehaviorTree> BTAsset;
};
