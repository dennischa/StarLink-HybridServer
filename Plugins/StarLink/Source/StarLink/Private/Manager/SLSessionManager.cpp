// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SLSessionManager.h"
#include "StarLink.h"
#include "StarLinkSubsystem.h"
#include "RPC/SLRPCHelperComponent.h"
#include "Session/SLSession.h"
#include "GameFramework/PlayerState.h"

USLSessionManager::USLSessionManager()
{
}

void USLSessionManager::Initialize()
{
	FStarLinkClientInfo ClientInfo;
	ClientInfo.ClientId = MASTER_CLIENT_ID;

	ConnectedClients.Add(ClientInfo.ClientId, ClientInfo);
}

void USLSessionManager::Deinitialize()
{
}

void USLSessionManager::RegisterClient(class APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		SL_NETWORK_LOG(PlayerController, LogStarLink, Warning, TEXT("RegisterClient: PlayerController is null"));
		return;
	}

	FStarLinkClientInfo ClientInfo;

	// PlayerState에서 고유 ID 추출
	if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
	{
		ClientInfo.ClientId = PlayerState->GetPlayerId();
	}
	else
	{
		SL_LOG(LogStarLink, Error, TEXT("RegisterClient: PlayerState is null"));
		return;
	}

	// NetConnection에서 IP 주소와 포트 추출
	if (UNetConnection* NetConnection = PlayerController->GetNetConnection())
	{
		ClientInfo.Url = NetConnection->URL;
	}
	
	if (auto* RPCComponent = PlayerController->FindComponentByClass<USLRPCHelperComponent>())
	{
		ClientInfo.RPCHelper = RPCComponent;
	}

	ClientInfo.Role = EStarLinkClientRole::None;

	// 클라이언트 정보 저장
	ConnectedClients.Add(ClientInfo.ClientId, ClientInfo);

	SL_NETWORK_LOG(PlayerController, LogStarLink, Log, TEXT("ConnectedClients Size: %d"), ConnectedClients.Num());

	SL_NETWORK_LOG(PlayerController, LogStarLink, Log, TEXT("Client Registered - ID: %d, Url: %s, Role: %d"),
		ClientInfo.ClientId, *ClientInfo.Url.ToString(), (int32)ClientInfo.Role);
}

void USLSessionManager::UnregisterClient(int32 ClientId)
{
	if (ConnectedClients.Remove(ClientId) > 0)
	{
		SL_LOG(LogStarLink, Log, TEXT("Client Unregistered - ID: %d"), ClientId);
	}
	else
	{
		SL_LOG(LogStarLink, Warning, TEXT("UnregisterClient: Client not found - ID: %d"), ClientId);
	}
}

FStarLinkClientInfo* USLSessionManager::GetClientInfo(int32 ClientId)
{
	return ConnectedClients.Find(ClientId);
}

USLSession* USLSessionManager::CreateSession(int32 ClientId)
{
	if (!ConnectedClients.Contains(ClientId))
	{
		SL_LOG(LogStarLink, Error, TEXT("Cannot Find Client Id : %d"), ClientId);
		return nullptr;
	}
	
	// 고유한 Session ID 생성
	const uint8 SessionId = ++SessionIdCounter;

	// Session 생성
	USLSession* NewSession = NewObject<USLSession>(this);

	// Session 초기화
	NewSession->Initialize(SessionId, ClientId, ConnectedClients);

	// Session 저장
	Sessions.Add(SessionId, NewSession);

	NewSession->Start();

	SL_LOG(LogStarLink, Log, TEXT("Session created - SessionId: %d, HostClientId: %d"),
		SessionId, ClientId);

	return NewSession;
}

void USLSessionManager::RemoveSession(int32 ClientId)
{
	USLSession* Session = nullptr;
	uint8 SessionId;
	
	for (auto SessionTuple : Sessions)
	{
		if (ClientId == SessionTuple.Value.Get()->GetHostClientId())
		{
			SessionId = SessionTuple.Key;
			Session = SessionTuple.Value.Get();
		}
	}

	if (!Session)
	{
		SL_LOG(LogStarLink, Error, TEXT("Cannot Find Session with Client Id: %d"), ClientId);
		return; 
	}

	if (const UStarLinkSubsystem* Subsystem = Cast<UStarLinkSubsystem>(GetOuter()))
	{
		if (auto RoleManager = Subsystem->GetRoleManager())
		{
			RoleManager->OnHandOver(SessionId);
		}
	}
	
	Sessions.Remove(SessionId);
}

USLSession* USLSessionManager::GetSession(uint8 SessionId)
{
	if (TObjectPtr<USLSession>* FoundSession = Sessions.Find(SessionId))
	{
		return *FoundSession;
	}
	return nullptr;
}
