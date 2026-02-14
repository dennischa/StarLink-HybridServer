// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StarLinkPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API AStarLinkPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AStarLinkPlayerController();

	UFUNCTION(Server, Reliable)
	void Server_Host(int32 PlayerId);

	UFUNCTION(Server, Reliable)
	void Server_Assign(uint32 NetGUID, int32 SessionId);

	UFUNCTION(Server, Reliable)
	void Server_Return(uint32 NetGUID);

	UFUNCTION(Exec)
	void StarLink(const FString& Command);

	int32 GetClientId() const { return ClientId; }

protected:
	void ExecuteHostCommand();
	void ExecuteAssignCommand(const TArray<FString>& CommandParts);
	void ExecuteReturnCommand(const TArray<FString>& CommandParts);
	void ExecuteBatchCommand(const TArray<FString>& CommandParts);

	virtual void BeginPlay() override;

	virtual void OnRep_PlayerState() override;

	UPROPERTY(VisibleAnywhere, Category="StarLink")
	TObjectPtr<class USLRPCHelperComponent> StarLinkRPCComponent;

	int32 ClientId;
};
