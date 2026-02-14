// Fill out your copyright notice in the Description page of Project Settings.


#include "Service/SLService.h"

#include "SLRoleActor.h"
#include "StarLink.h"
#include "Engine/PackageMapClient.h"
#include "Session/SLClient.h"
#include "RPC/SLRPC.h"
#include "RPC/SLRPCResponse.h"

USLService::USLService()
{
	
}

void USLService::Initialize()
{
}

void USLService::Tick(float DeltaTime)
{
	if (HostClient)
	{
		HostClient->Tick(DeltaTime);
	}

	for (auto PeerClient : PeerClients)
	{
		if (PeerClient.Value)
		{
			PeerClient.Value->Tick(DeltaTime);
		}
	}
}

FSLRPCResponseData USLService::CreateHostClient(const FString& SessionId, const FURL& InURL)
{
	FSLRPCResponseData ResponseData;
	
	USLClient* Client = NewObject<USLClient>(this);

	if (Client->InitializeAsHost(InURL, SessionId))
	{
		HostSessionId = SessionId;
		HostClient = Client;

		ResponseData.SetSuccess(TEXT("Host Set"));
	}
	else
	{
		ResponseData.SetFailed(TEXT("Failed to Initialize Host Client"), 10001);
	}

	return ResponseData;
}

TSharedPtr<FSLRPCResponse> USLService::CreatePeerClient(uint8 SessionId, const FURL& InURL)
{
	USLClient* Client = NewObject<USLClient>(this);
	
	PeerClients.Add(SessionId, Client);

	TSharedPtr<FSLRPCResponse> Response = MakeShared<FSLRPCResponse>();

	Client->InitializeAsPeer(InURL, SessionId, SLClientInitializeComplete::CreateLambda([this, Response](bool bResult, FString Message)
	{
		if (bResult)
		{
			if (Response)
			{
				Response->SetSuccess(Message);
			}
		}
		else
		{
			Response->SetFailed(TEXT("Failed to Initialize Peer"), 10002);
		}
	}));
	
	return Response;
}

TWeakObjectPtr<USLClient> USLService::GetPeerClient(uint8 SessionId)
{
	return PeerClients[SessionId];
}

FSLRPCResponseData USLService::Assign(uint32 GUID)
{
	FSLRPCResponseData ResponseData;
	
	if (!HostClient)
	{
		ResponseData.SetFailed(TEXT("The Host Client has not been initialized"), SLRPC_ERROR_CODE_FAILED_FIND_HOST);
		return ResponseData;
	}

	UNetDriver* NetDriver = HostClient->GetNetDriver();

	AActor* TargetActor = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(GUID, true));

	if (!TargetActor)
	{
		ResponseData.SetFailed(TEXT("Failed to find Target Actor"), SLRPC_ERROR_CODE_FAILED_FIND_ACTOR);
		return ResponseData;
	}

	ISLRoleActor* RoleActor = Cast<ISLRoleActor>(TargetActor);
	// 권한 부여
	TargetActor->SetRole(ROLE_Authority);

	// Host Net Driver 등록
	TargetActor->SetNetDriverName(HostClient->GetNetDriverName());

	//NetDriver->AddNetworkActor(TargetActor);

	if (RoleActor)
	{
		RoleActor->Host_OnAssign();
	}

	ResponseData.SetSuccess(FString::Printf(TEXT("Succeed to Assign GUID: %u"), GUID));
	
	NetDriver->DebugRelevantActors = true;
	
	return ResponseData;
}

FSLRPCResponseData USLService::Return(uint32 GUID)
{
	FSLRPCResponseData ResponseData;
	
	if (!HostClient)
	{
		ResponseData.SetFailed(TEXT("The Host Client has not been initialized"), SLRPC_ERROR_CODE_FAILED_FIND_HOST);
		return ResponseData;
	}

	UNetDriver* HostNetDriver = HostClient->GetNetDriver();
	UNetDriver* GameNetDriver = GetWorld()->GetNetDriver();

	AActor* TargetActor = Cast<AActor>(HostNetDriver->GuidCache->GetObjectFromNetGUID(GUID, true));

	if (!TargetActor)
	{
		ResponseData.SetFailed(TEXT("Failed to find Target Actor"), SLRPC_ERROR_CODE_FAILED_FIND_ACTOR);
		return ResponseData;
	}

	ISLRoleActor* RoleActor = Cast<ISLRoleActor>(TargetActor);
	
	if (RoleActor)
	{
		RoleActor->Host_OnReturn();
	}
	
	// GameNetDriver 복원
	TargetActor->SetNetDriverName(GameNetDriver->NetDriverName);
	// 권한 제거
	TargetActor->SetRole(ROLE_SimulatedProxy);
	
	HostNetDriver->RemoveNetworkActor(TargetActor);
	
	ResponseData.SetSuccess(FString::Printf(TEXT("Succeed to Assign GUID: %u"), GUID));

	SL_NETWORK_LOG(TargetActor, LogStarLink, Log, TEXT("Hello"));

	HostNetDriver->DebugRelevantActors = true;
	
	return ResponseData;
}
