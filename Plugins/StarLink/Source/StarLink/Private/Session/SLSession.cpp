// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/SLSession.h"

#include "SLRoleActor.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetDriver.h"
#include "Session/SLClient.h"
#include "StarLink.h"
#include "StarLinkSubsystem.h"
#include "Engine/PackageMapClient.h"
#include "RPC/SLRPC.h"
#include "RPC/SLRPCHelperComponent.h"
#include "Service/SLService.h"

void USLSession::Initialize(uint8 InSessionId, int32 InHostClientId, TMap<int32, FStarLinkClientInfo> Clients)
{
	SessionId = InSessionId;
	HostClientId = InHostClientId;

	HostInfo = Clients[InHostClientId];
	HostInfo.Role = EStarLinkClientRole::Host;

	HostInfo.Url.Protocol = TEXT("unreal");
	HostInfo.Url.Port = HOST_CLIENT_PORT + InSessionId;

	Clients.Remove(InHostClientId);

	PeerClients = Clients;

	SL_LOG(LogStarLink, Log, TEXT("Session Initialized - SessionId: %d, HostClientId: %d"),
		SessionId, HostClientId);
}

void USLSession::OnStateChange(ESLSessionState State, FString Message)
{
	CurrentState = State;

	SL_LOG(LogStarLink, Log, TEXT("Session State Change[%s]: %s"), *EnumToString(State), *Message);

	OnStateChangeDel.Broadcast(State, Message);
}

void USLSession::AddClient(const FStarLinkClientInfo& ClientInfo)
{
	// 이미 존재하는지 확인
	if (PeerClients.Contains(ClientInfo.ClientId))
	{
		SL_LOG(LogStarLink, Warning, TEXT("Client already exists in session - ClientId: %d"), ClientInfo.ClientId);
		return;
	}

	PeerClients.Add(ClientInfo.ClientId, ClientInfo);
	
	SL_LOG(LogStarLink, Log, TEXT("Client added to session - SessionId: %d, ClientId: %d"),
		SessionId, ClientInfo.ClientId);
}

void USLSession::RemoveClient(int32 ClientId)
{
	if (PeerClients.Remove(ClientId) > 0)
	{
		SL_LOG(LogStarLink, Log, TEXT("Client removed from session - SessionId: %d, ClientId: %d"),
			SessionId, ClientId);
	}
	else
	{
		SL_LOG(LogStarLink, Warning, TEXT("Client not found in session - SessionId: %d, ClientId: %d"),
			SessionId, ClientId);
	}
}

void USLSession::Start()
{
	SL_LOG(LogStarLink, Log, TEXT("Session started"));
	RequestHostClient();
}

void USLSession::Assign(AActor* AssignableActor)
{
	if (AssignableActor == nullptr)
	{
		SL_LOG(LogStarLink, Error, TEXT("Actor is null"));
		return;
	}
	
	auto* SLAssignableActor = Cast<ISLRoleActor>(AssignableActor);
	
	if (!SLAssignableActor)
	{
		SL_NETWORK_LOG(AssignableActor, LogStarLink, Error, TEXT("Failed to Cast SLAssignableActor"))
		return;
	}
	
	SLAssignableActor->Master_OnAssign();
	
	// 권한 제거
	AssignableActor->SetRole(ROLE_SimulatedProxy);
	
	FString GUID;
	if (auto NetDriver = GetWorld()->GetNetDriver())
	{
		NetDriver->RemoveNetworkActor(AssignableActor);
		
		FNetworkGUID NetworkGUID = NetDriver->GuidCache->GetOrAssignNetGUID(AssignableActor);
		GUID = LexToString(NetworkGUID.Value);
	}
	
	if (HostInfo.RPCHelper.IsValid())
	{
		USLRPCHelperComponent* RPCHelperComponent = HostInfo.RPCHelper.Get();

		TMap<FString, FString> Params;
		Params.Add(SLRPC_PARAM_ACTOR_GUID,GUID);
		
		TFuture<FSLRPCResponseData> Future = RPCHelperComponent->CallClientRPCWithResponse(SLRPC_ASSIGN, Params);
		
		Future.Then([this](const TFuture<FSLRPCResponseData>& ResponseFuture)
		{
			if (ResponseFuture.IsReady())
			{
				auto Response = ResponseFuture.Get();

				if (Response.IsSuccess())
				{
					SL_LOG(LogStarLink, Log, TEXT("Assign successfully, GUID: %s"), *Response.Message);
				}
				else
				{
					SL_LOG(LogStarLink, Error, TEXT("Failed: %s (ErrorCode: %d)"), *Response.Message, Response.ErrorCode);
				}
			}
		}); 
	}
}

void USLSession::Return(AActor* InActor)
{
	if (InActor == nullptr)
	{
		SL_LOG(LogStarLink, Error, TEXT("Actor is null"));
		return;
	}
	
	auto GameNetDriver = GetWorld()->GetNetDriver();
	
	FNetworkGUID NetworkGUID = GameNetDriver->GuidCache->GetOrAssignNetGUID(InActor);
	FString GUIDString;
	GUIDString = LexToString(NetworkGUID.Value);
	
	if (HostInfo.RPCHelper.IsValid())
	{
		USLRPCHelperComponent* RPCHelperComponent = HostInfo.RPCHelper.Get();

		TMap<FString, FString> Params;
		Params.Add(SLRPC_PARAM_ACTOR_GUID,GUIDString);
		
		TFuture<FSLRPCResponseData> Future = RPCHelperComponent->CallClientRPCWithResponse(SLRPC_RETURN, Params);
		
		Future.Then([this, InActor, GUIDString](const TFuture<FSLRPCResponseData>& ResponseFuture)
		{
			if (ResponseFuture.IsReady())
			{
				auto Response = ResponseFuture.Get();

				if (Response.IsSuccess())
				{
					SL_LOG(LogStarLink, Log, TEXT("Return successfully, GUID: %s"), *GUIDString);
					if (InActor)
					{
						Master_OnReturn(InActor);
					}
				}
				else
				{
					SL_LOG(LogStarLink, Error, TEXT("Failed: %s (ErrorCode: %d)"), *Response.Message, Response.ErrorCode);
				}
			}
		}); 
	}
}

void USLSession::Master_OnReturn(AActor* InActor)
{
	auto* SLRoleActor = Cast<ISLRoleActor>(InActor);
	
	if (!SLRoleActor)
	{
		SL_NETWORK_LOG(InActor, LogStarLink, Error, TEXT("Failed to Cast SLRoleActor"))
		return;
	}

	auto GameNetDriver = GetWorld()->GetNetDriver();

	// ★ 기존 ActorChannel을 닫아서 캐시 초기화
	// (닫지 않으면 이전 값과 현재 값이 동일하여 변경 감지 안됨)
	for (UNetConnection* Conn : GameNetDriver->ClientConnections)
	{
		if (Conn)
		{
			UActorChannel* Channel = Conn->FindActorChannelRef(InActor);
			if (Channel)
			{
				Channel->Close(EChannelCloseReason::Relevancy);
			}
		}
	}

	// 권한 부여
	InActor->SetRole(ROLE_Authority);

	InActor->SetNetDriverName(GameNetDriver->NetDriverName);

	GameNetDriver->AddNetworkActor(InActor);

	SLRoleActor->Master_OnReturn();

	// 즉시 리플리케이션 스케줄링
	InActor->ForceNetUpdate();

	SL_NETWORK_LOG(InActor, LogStarLink, Log, TEXT("Master_OnReturn completed"))
}

void USLSession::RequestHostClient()
{
	if (CurrentState != ESLSessionState::Idle)
	{
		SL_LOG(LogStarLink, Error, TEXT("Current State: %s"), *UEnum::GetValueAsString<ESLSessionState>(CurrentState));	
		return;
	}
	
	if (HostInfo.RPCHelper.IsValid())
	{
		USLRPCHelperComponent* RPCHelperComponent = HostInfo.RPCHelper.Get();

		TMap<FString, FString> Params;
		Params.Add(SLRPC_PARAM_SESSION_ID, FString::FromInt(SessionId));
		Params.Add(SLRPC_PARAM_Port, FString::FromInt(HostInfo.Url.Port));
		
		TFuture<FSLRPCResponseData> Future = RPCHelperComponent->CallClientRPCWithResponse(SLRPC_CREATE_HOST, Params);
		OnStateChange(ESLSessionState::RequestHostClient);
		
		Future.Then([this](TFuture<FSLRPCResponseData> ResponseFuture)
		{
			if (ResponseFuture.IsReady())
			{
				auto Response = ResponseFuture.Get();

				if (Response.IsSuccess())
				{
					TMap<FString, FString> Data = USLRPCHelperComponent::ArrayToMap(Response.Data);
					SL_LOG(LogStarLink, Log, TEXT("Host response successfully"));
					OnStateChange(ESLSessionState::ConnectMasterClient);
					ConnectMasterClient();
				}
				else
				{
					CurrentState = ESLSessionState::Failed;
					SL_LOG(LogStarLink, Error, TEXT("Failed: %s (ErrorCode: %d)"), *Response.Message, Response.ErrorCode);
				}
			}
		}); 
	}
}

void USLSession::ConnectMasterClient()
{
	if (CurrentState != ESLSessionState::ConnectMasterClient)
	{
		OnStateChange(ESLSessionState::Failed, FString::Format(TEXT("Wrong State : %s"), {*EnumToString(CurrentState)}));
		return;
	}
	
	auto* SLService = UStarLinkSubsystem::Get(this)->GetSLService();

	auto Response = SLService->CreatePeerClient(SessionId, HostInfo.Url);

	Response->GetFuture().Then([this](const TFuture<FSLRPCResponseData>& ResponseFuture)
	{
		if (ResponseFuture.IsReady())
		{
			auto Response = ResponseFuture.Get();
			if (Response.IsSuccess())
			{
				// Success
				SL_LOG(LogStarLink, Log, TEXT("MASTER Client Successfully connected to server"));

				if (PeerClients.Find(MASTER_CLIENT_ID))
				{
					PeerClients[MASTER_CLIENT_ID].bIsReady = true;
				}
				
				ConnectPeerClients();
			}
			else
			{
				// Fail
				SL_LOG(LogStarLink, Log, TEXT("Failed: %s (ErrorCode: %d)"), *Response.Message, Response.ErrorCode);
			}
		}
	});
}

void USLSession::ConnectPeerClients()
{
	OnStateChange(ESLSessionState::ConnectPeerClients);
	
	for (auto& PeerClient : PeerClients)
	{
		if (PeerClient.Value.ClientId == MASTER_CLIENT_ID)
			continue;

		if (PeerClient.Value.RPCHelper.IsValid())
		{
			USLRPCHelperComponent* RPCHelperComponent = PeerClient.Value.RPCHelper.Get();

			TMap<FString, FString> Params;
			Params.Add(SLRPC_PARAM_SESSION_ID, FString::FromInt(SessionId));
			Params.Add(SLRPC_PARAM_HOST, HostInfo.Url.Host);
			Params.Add(SLRPC_PARAM_Port, FString::FromInt(HostInfo.Url.Port));
			
			TFuture<FSLRPCResponseData> Future = RPCHelperComponent->CallClientRPCWithResponse(SLRPC_CONNECT_PEER, Params);
			
			Future.Then([this, Key = PeerClient.Key](TFuture<FSLRPCResponseData> ResponseFuture)
			{
				if (ResponseFuture.IsReady())
				{
					auto Response = ResponseFuture.Get();

					if (Response.IsSuccess())
					{
						SL_LOG(LogStarLink, Log, TEXT("Client-%d Successfully connected to server"), Key);
						PeerClients[Key].bIsReady = true;
						OnConnectPeerClient();
					}
					else
					{
						OnStateChange(ESLSessionState::Failed);
						SL_LOG(LogStarLink, Error, TEXT("Failed: %s (ErrorCode: %d)"), *Response.Message, Response.ErrorCode);
					}
				}
			}); 
		}
	}
}

void USLSession::OnConnectPeerClient()
{
	for (auto& PeerClient : PeerClients)
	{
		if (!PeerClient.Value.bIsReady)
			return;
	}

	OnStateChange(ESLSessionState::Completed);
}

