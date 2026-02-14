// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/StarLinkPlayerController.h"

#include "MythicDungeon.h"
#include "StarLink.h"
#include "StarLinkSubsystem.h"
#include "Character/MDCharacterNonPlayer.h"
#include "Engine/PackageMapClient.h"
#include "GameFramework/PlayerState.h"
#include "Manager/SLRoleManager.h"
#include "Manager/SLSessionManager.h"
#include "RPC/SLRPCHelperComponent.h"

AStarLinkPlayerController::AStarLinkPlayerController()
{
	StarLinkRPCComponent = CreateDefaultSubobject<USLRPCHelperComponent>(TEXT("StarLinkRPC"));
}

void AStarLinkPlayerController::Server_Host_Implementation(int32 InClientId)
{
	auto* StarLink = GetGameInstance()->GetSubsystem<UStarLinkSubsystem>();

	if (auto* SessionManager = StarLink->GetSessionManager())
	{
		auto Clients = SessionManager->GetAllClients();

		if (Clients.Contains(InClientId))
		{
			SessionManager->CreateSession(InClientId);
		}
	}
}

void AStarLinkPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);
}

void AStarLinkPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
	{
		ClientId = PlayerState->GetPlayerId();
	}
}

void AStarLinkPlayerController::Server_Assign_Implementation(uint32 NetGUID, int32 SessionId)
{
	auto* StarLink = GetGameInstance()->GetSubsystem<UStarLinkSubsystem>();

	if (auto* RoleManager = StarLink->GetRoleManager())
	{
		AActor* Actor = Cast<AActor>(GetNetDriver()->GuidCache->GetObjectFromNetGUID(NetGUID, true));
		
		RoleManager->Assign(Actor, SessionId);
	}
}

void AStarLinkPlayerController::Server_Return_Implementation(uint32 NetGUID)
{
	auto* StarLink = GetGameInstance()->GetSubsystem<UStarLinkSubsystem>();

	if (auto* RoleManager = StarLink->GetRoleManager())
	{
		AActor* Actor = Cast<AActor>(GetNetDriver()->GuidCache->GetObjectFromNetGUID(NetGUID, true));
		
		RoleManager->Return(Actor);
	}
}

void AStarLinkPlayerController::StarLink(const FString& Command)
{
	if (!StarLinkRPCComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("StarLinkRPCComponent is null"));
		return;
	}

	TArray<FString> CommandParts;
	Command.ParseIntoArray(CommandParts, TEXT(" "), true);

	if (CommandParts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Empty StarLink command"));
		return;
	}

	const FString CommandType = CommandParts[0];

	if (CommandType.Equals(TEXT("host"), ESearchCase::IgnoreCase))
	{
		ExecuteHostCommand();
	}
	else if (CommandType.Equals(TEXT("assign"), ESearchCase::IgnoreCase))
	{
		ExecuteAssignCommand(CommandParts);
	}
	else if (CommandType.Equals(TEXT("return"), ESearchCase::IgnoreCase))
	{
		ExecuteReturnCommand(CommandParts);
	}
	else if (CommandType.Equals(TEXT("batch"), ESearchCase::IgnoreCase))
	{
		ExecuteBatchCommand(CommandParts);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown StarLink command: %s"), *CommandType);
	}
}

void AStarLinkPlayerController::ExecuteHostCommand()
{
	Server_Host(ClientId);
}

void AStarLinkPlayerController::ExecuteAssignCommand(const TArray<FString>& CommandParts)
{
	if (CommandParts.Num() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: starlink assign <netguid> <sessionid>"));
		return;
	}

	uint32 NetGUIDValue = FCString::Atoi(*CommandParts[1]);
	int32 SessionId = FCString::Atoi(*CommandParts[2]);

	UE_LOG(LogTemp, Log, TEXT("StarLink Assign Command - NetGUID: %d, SessionID: %d"),
		NetGUIDValue, SessionId);

	Server_Assign(NetGUIDValue, SessionId);
}

void AStarLinkPlayerController::ExecuteReturnCommand(const TArray<FString>& CommandParts)
{
	if (CommandParts.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: starlink return <netguid>"));
		return;
	}

	uint32 NetGUIDValue = FCString::Atoi(*CommandParts[1]);

	Server_Return(NetGUIDValue);
}

void AStarLinkPlayerController::ExecuteBatchCommand(const TArray<FString>& CommandParts)
{
	if (CommandParts.Num() < 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: starlink batch <start> <end> <sessionid>"));
		return;
	}

	uint32 Start = FCString::Atoi(*CommandParts[1]);
	uint32 End = FCString::Atoi(*CommandParts[2]);
	int32 SessionId = FCString::Atoi(*CommandParts[3]);

	if (Start % 2 != 0)
	{
		Start += 1;
	}

	for (uint32 NetGUIDValue = Start; NetGUIDValue <= End; NetGUIDValue += 2)
	{
		UE_LOG(LogTemp, Log, TEXT("StarLink Batch Assign - NetGUID: %d, SessionID: %d"),
			NetGUIDValue, SessionId);

		Server_Assign(NetGUIDValue, SessionId);
	}
}
