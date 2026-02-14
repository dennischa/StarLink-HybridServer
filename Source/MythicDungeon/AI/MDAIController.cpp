// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MDAIController.h"

#include "MDAI.h"
#include "MythicDungeon.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AMDAIController::AMDAIController()
{
	bReplicates = true;
	
	bOnlyRelevantToOwner = false;

	bAlwaysRelevant = true;
}

void AMDAIController::RunAI()
{
	UBlackboardComponent* BlackboardPtr = Blackboard.Get();
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		Blackboard->SetValueAsVector(BBKEY_HOMEPOS, GetPawn()->GetActorLocation());

		bool RunResult = RunBehaviorTree(BTAsset);
		ensure(RunResult);
	}
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);

	MD_NETWORK_LOG(LogMD, Log, TEXT("RunAI: %d"), BTComponent->IsRunning());
}

void AMDAIController::StopAI()
{
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}

	MD_NETWORK_LOG(LogMD, Log, TEXT("StopAI"));
}

void AMDAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RunAI();
}