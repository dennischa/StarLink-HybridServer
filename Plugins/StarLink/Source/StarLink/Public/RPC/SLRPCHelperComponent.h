// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPC/SLRPCResponse.h"
#include "SLRPCHelperComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STARLINK_API USLRPCHelperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLRPCHelperComponent();

	// ========== TFuture 기반 RPC 시스템 ==========

	// 클라이언트 -> 서버: 요청을 보내고 응답을 TFuture로 받음
	TFuture<FSLRPCResponseData> CallServerRPCWithResponse(const FString& RPCName, const TMap<FString, FString>& Params = TMap<FString, FString>());

	// 서버 -> 클라이언트: 요청을 보내고 응답을 TFuture로 받음
	TFuture<FSLRPCResponseData> CallClientRPCWithResponse(const FString& RPCName, const TMap<FString, FString>& Params = TMap<FString, FString>());

	// 서버: 클라이언트로부터 받은 요청 처리
	UFUNCTION(Server, Reliable)
	void ServerRPC_Request(const FString& RequestId, const FString& RPCName, const TArray<FSLRPCParam>& Params);

	// 클라이언트: 서버로부터 받은 요청 처리
	UFUNCTION(Client, Reliable)
	void ClientRPC_Request(const FString& RequestId, const FString& RPCName, const TArray<FSLRPCParam>& Params);
	
	// 클라이언트: 서버에게 응답 전송
	UFUNCTION(Server, Reliable)
	void ServerRPC_Response(const FString& RequestId, const FSLRPCResponseData& ResponseData);

	// 서버: 클라이언트에게 응답 전송
	UFUNCTION(Client, Reliable)
	void ClientRPC_Response(const FString& RequestId, const FSLRPCResponseData& ResponseData);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	
	// Map <-> Array 변환 헬퍼 함수
	static TArray<FSLRPCParam> MapToArray(const TMap<FString, FString>& Map);
	static TMap<FString, FString> ArrayToMap(const TArray<FSLRPCParam>& Array);
	
private:
	// 고유한 요청 ID 생성
	FString GenerateRequestId();
	
	// 요청 ID -> Promise 맵핑 (응답 대기 중인 요청들)
	TMap<FString, TSharedPtr<FSLRPCResponse>> PendingRequests;

	// 요청 ID 카운터
	uint64 RequestIdCounter = 0;

	// Critical Section for thread safety
	FCriticalSection RequestMapLock;

	UPROPERTY()
	TObjectPtr<class UStarLinkSubsystem> StarLinkSubsystem;
};
