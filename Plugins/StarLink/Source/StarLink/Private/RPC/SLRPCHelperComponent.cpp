// Fill out your copyright notice in the Description page of Project Settings.


#include "RPC/SLRPCHelperComponent.h"

#include "StarLink.h"
#include "StarLinkSubsystem.h"
#include "Service/SLService.h"
#include "RPC/SLRPC.h"

USLRPCHelperComponent::USLRPCHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USLRPCHelperComponent::BeginPlay()
{
	Super::BeginPlay();

	StarLinkSubsystem = UStarLinkSubsystem::Get(this);
}

FString USLRPCHelperComponent::GenerateRequestId()
{
	FScopeLock Lock(&RequestMapLock);
	RequestIdCounter++;
	return FString::Printf(TEXT("%s_%llu"), *GetOwner()->GetName(), RequestIdCounter);
}

TArray<FSLRPCParam> USLRPCHelperComponent::MapToArray(const TMap<FString, FString>& Map)
{
	TArray<FSLRPCParam> Result;
	for (const auto& Pair : Map)
	{
		Result.Add(FSLRPCParam(Pair.Key, Pair.Value));
	}
	return Result;
}

TMap<FString, FString> USLRPCHelperComponent::ArrayToMap(const TArray<FSLRPCParam>& Array)
{
	TMap<FString, FString> Result;
	for (const FSLRPCParam& Param : Array)
	{
		Result.Add(Param.Key, Param.Value);
	}
	return Result;
}

TFuture<FSLRPCResponseData> USLRPCHelperComponent::CallServerRPCWithResponse(const FString& RPCName, const TMap<FString, FString>& Params)
{
	// 고유 요청 ID 생성
	FString RequestId = GenerateRequestId();

	// Promise 생성 및 저장
	TSharedPtr<FSLRPCResponse> Response = MakeShared<FSLRPCResponse>();
	{
		FScopeLock Lock(&RequestMapLock);
		PendingRequests.Add(RequestId, Response);
	}

	// Map을 Array로 변환 후 서버로 요청 전송
	ServerRPC_Request(RequestId, RPCName, MapToArray(Params));

	SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("CallServerRPCWithResponse - RequestId: %s, RPCName: %s"), *RequestId, *RPCName);

	// Future 반환
	return Response->GetFuture();
}

TFuture<FSLRPCResponseData> USLRPCHelperComponent::CallClientRPCWithResponse(const FString& RPCName, const TMap<FString, FString>& Params)
{
	// 고유 요청 ID 생성
	FString RequestId = GenerateRequestId();

	// Promise 생성 및 저장
	TSharedPtr<FSLRPCResponse> Response = MakeShared<FSLRPCResponse>();
	
	{
		FScopeLock Lock(&RequestMapLock);
		PendingRequests.Add(RequestId, Response);
	}

	// Map을 Array로 변환 후 클라이언트로 요청 전송
	ClientRPC_Request(RequestId, RPCName, MapToArray(Params));

	SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("CallClientRPCWithResponse - RequestId: %s, RPCName: %s"), *RequestId, *RPCName);

	// Future 반환
	return Response->GetFuture();
}

void USLRPCHelperComponent::ServerRPC_Request_Implementation(const FString& RequestId, const FString& RPCName, const TArray<FSLRPCParam>& Params)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("ServerRPC_Request - RequestId: %s, RPCName: %s"), *RequestId, *RPCName);

	TMap<FString, FString> ParamsMap = ArrayToMap(Params);

	// 여기서 실제 RPC 로직을 처리하고 응답을 생성합니다.
	// 예제: RPCName에 따라 다른 처리를 할 수 있습니다.
	FSLRPCResponseData ResponseData;

	if (RPCName == TEXT("Hello"))
	{
		ResponseData.Status = ESLRPCResponseStatus::Success;
		ResponseData.Message = TEXT("Hello from Server!");
		ResponseData.Data.Add(FSLRPCParam(TEXT("ServerTime"), FString::Printf(TEXT("%f"), GetWorld()->GetTimeSeconds())));
	}
	else if (RPCName == TEXT("GetPlayerInfo"))
	{
		ResponseData.Status = ESLRPCResponseStatus::Success;
		ResponseData.Message = TEXT("Player info retrieved");
		ResponseData.Data.Add(FSLRPCParam(TEXT("PlayerName"), GetOwner()->GetName()));
		ResponseData.Data.Add(FSLRPCParam(TEXT("Role"), GetOwnerRole() == ROLE_Authority ? TEXT("Server") : TEXT("Client")));
	}
	else
	{
		// 알 수 없는 RPC
		ResponseData.Status = ESLRPCResponseStatus::Failed;
		ResponseData.Message = FString::Printf(TEXT("Unknown RPC: %s"), *RPCName);
		ResponseData.ErrorCode = 404;
	}

	// 클라이언트에게 응답 전송
	ClientRPC_Response(RequestId, ResponseData);
}

void USLRPCHelperComponent::ClientRPC_Request_Implementation(const FString& RequestId, const FString& RPCName, const TArray<FSLRPCParam>& Params)
{
	if (GetOwnerRole() != ROLE_AutonomousProxy)
		return;

	SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("ClientRPC_Request - RequestId: %s, RPCName: %s"), *RequestId, *RPCName);

	TMap<FString, FString> ParamsMap = ArrayToMap(Params);
	FSLRPCResponseData ResponseData;

	if (RPCName.Equals(SLRPC_CREATE_HOST, ESearchCase::IgnoreCase))
	{
		FString SessionId = ParamsMap[SLRPC_PARAM_SESSION_ID];
		int32 Port;
		LexTryParseString(Port, *ParamsMap[SLRPC_PARAM_Port]);

		if (auto SLService = StarLinkSubsystem->GetSLService())
		{
			FURL URL;

			URL.Protocol = TEXT("unreal");
			URL.Port = Port;
		
			ResponseData  = SLService->CreateHostClient(SessionId, URL);
			ServerRPC_Response(RequestId, ResponseData);
		}
	}
	else if (RPCName.Equals(SLRPC_CONNECT_PEER, ESearchCase::IgnoreCase))
	{
		uint8 SessionId = FCString::Atoi(*ParamsMap[SLRPC_PARAM_SESSION_ID]);
		FString Host = ParamsMap[SLRPC_PARAM_HOST];
		int32 Port;
		LexTryParseString(Port, *ParamsMap[SLRPC_PARAM_Port]);
		
		if (auto SLService = StarLinkSubsystem->GetSLService())
		{
			FURL URL;
			URL.Protocol = TEXT("unreal");
			URL.Host = Host;
			URL.Port = Port;

			auto Response = SLService->CreatePeerClient(SessionId, URL);
		
			Response->GetFuture().Then([this, RequestId](const TFuture<FSLRPCResponseData>& ResponseFuture)
			{
				if (ResponseFuture.IsReady())
				{
					auto Response = ResponseFuture.Get();
					ServerRPC_Response(RequestId, Response);
				}
			});
		}
	}
	else if (RPCName.Equals(SLRPC_ASSIGN, ESearchCase::IgnoreCase))
	{
		uint32 GUID = 0;
		LexTryParseString(GUID, *ParamsMap[SLRPC_PARAM_ACTOR_GUID]);

		if (auto SLService = StarLinkSubsystem->GetSLService())
		{
			ResponseData = SLService->Assign(GUID);
		}
		else
		{
			ResponseData.SetFailed(TEXT("Could not parse ActorGUID"), SLRPC_ERROR_CODE_INVALID_PARAM);
		}
		
		ServerRPC_Response(RequestId, ResponseData);
	}
	else if (RPCName.Equals(SLRPC_RETURN, ESearchCase::IgnoreCase))
	{
		uint32 GUID = 0;
		LexTryParseString(GUID, *ParamsMap[SLRPC_PARAM_ACTOR_GUID]);

		if (auto SLService = StarLinkSubsystem->GetSLService())
		{
			ResponseData = SLService->Return(GUID);
		}
		else
		{
			ResponseData.SetFailed(TEXT("Could not parse ActorGUID"), SLRPC_ERROR_CODE_INVALID_PARAM);
		}
		
		ServerRPC_Response(RequestId, ResponseData);
	}
	else
	{
		// 알 수 없는 RPC
		ResponseData.Status = ESLRPCResponseStatus::Failed;
		ResponseData.Message = FString::Printf(TEXT("Unknown RPC: %s"), *RPCName);
		ResponseData.ErrorCode = 404;
		
		ServerRPC_Response(RequestId, ResponseData);
	}
}

void USLRPCHelperComponent::ClientRPC_Response_Implementation(const FString& RequestId, const FSLRPCResponseData& ResponseData)
{
	if (GetOwnerRole() == ROLE_Authority)
		return;

	SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("ClientRPC_Response - RequestId: %s, Status: %d, Message: %s"),
		*RequestId, (int32)ResponseData.Status, *ResponseData.Message);

	// 대기 중인 Promise를 찾아서 값을 설정
	TSharedPtr<FSLRPCResponse> Response;
	{
		FScopeLock Lock(&RequestMapLock);
		if (PendingRequests.RemoveAndCopyValue(RequestId, Response))
		{
			if (Response.IsValid())
			{
				Response->SetValue(ResponseData);
				SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("ClientRPC_Response - Promise completed for RequestId: %s"), *RequestId);
			}
		}
		else
		{
			SL_NETWORK_LOG(GetOwner(), LogStarLink, Warning, TEXT("ClientRPC_Response - No pending request found for RequestId: %s"), *RequestId);
		}
	}
}

void USLRPCHelperComponent::ServerRPC_Response_Implementation(const FString& RequestId, const FSLRPCResponseData& ResponseData)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("ServerRPC_Response - RequestId: %s, Status: %d, Message: %s"),
		*RequestId, (int32)ResponseData.Status, *ResponseData.Message);

	// 대기 중인 Promise를 찾아서 값을 설정
	TSharedPtr<FSLRPCResponse> Response;
	{
		FScopeLock Lock(&RequestMapLock);
		if (PendingRequests.RemoveAndCopyValue(RequestId, Response))
		{
			if (Response.IsValid())
			{
				Response->SetValue(ResponseData);
				SL_NETWORK_LOG(GetOwner(), LogStarLink, Log, TEXT("ServerRPC_Response - Promise completed for RequestId: %s"), *RequestId);
			}
		}
		else
		{
			SL_NETWORK_LOG(GetOwner(), LogStarLink, Warning, TEXT("ServerRPC_Response - No pending request found for RequestId: %s"), *RequestId);
		}
	}
}

