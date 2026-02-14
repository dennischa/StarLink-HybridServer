   // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StarLinkTypes.h"
#include "SLSession.generated.h"

#define MASTER_CLIENT_ID 10000
#define HOST_CLIENT_PORT 17780


UENUM()
enum class ESLSessionState
{
	Idle,
	RequestHostClient,
	ConnectMasterClient,
	ConnectPeerClients,
	Completed,
	Failed
};

inline FString EnumToString(ESLSessionState Type)
{
	return StaticEnum<ESLSessionState>()->GetNameStringByValue((int64)Type);
}

DECLARE_MULTICAST_DELEGATE_TwoParams(OnSLSessionStateChange, ESLSessionState, FString);

/**
 * StarLink Session - 클라이언트들을 관리하는 세션
 */
UCLASS()
class STARLINK_API USLSession : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Session을 초기화하고 Host Client를 설정합니다.
	 * @param InSessionId Session의 고유 ID
	 * @param InHostClient Host로 설정할 클라이언트 정보
	 */
	void Initialize(uint8 InSessionId, int32 InHostClientId, TMap<int32, FStarLinkClientInfo> Clients);

	/**
	 * Session에 새로운 클라이언트를 추가합니다.
	 * @param ClientInfo 추가할 클라이언트 정보
	 */
	void AddClient(const FStarLinkClientInfo& ClientInfo);

	/**
	 * Session에서 클라이언트를 제거합니다.
	 * @param ClientId 제거할 클라이언트 ID
	 */
	void RemoveClient(int32 ClientId);

	uint8 GetSessionId() const { return SessionId; }
	int32 GetHostClientId() const { return HostClientId; }
	
	void Start();

	OnSLSessionStateChange OnStateChangeDel;

	void Assign(AActor* AssignableActor);

	void Return(AActor* InActor);
	
	void Master_OnReturn(AActor* InActor);
	
protected:
	void RequestHostClient();

	void ConnectMasterClient();

	void ConnectPeerClients();

	void OnConnectPeerClient();

	void OnStateChange(ESLSessionState State, FString Message = TEXT(""));

protected:
	UPROPERTY()
	ESLSessionState CurrentState = ESLSessionState::Idle;
	
	uint8 SessionId;

	int32 HostClientId;

	UPROPERTY()
	FStarLinkClientInfo HostInfo;

	UPROPERTY()
	TMap<int32, FStarLinkClientInfo> PeerClients;
};