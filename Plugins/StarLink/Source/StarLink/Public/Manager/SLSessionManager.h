// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLRoleManager.h"
#include "StarLinkSubsystem.h"
#include "StarLinkTypes.h"
#include "SLSessionManager.generated.h"

class USLSession;

/**
 * StarLink MasterManager - 서버에서 Session들을 관리
 */
UCLASS()
class STARLINK_API USLSessionManager : public UObject
{
	GENERATED_BODY()

public:
	USLSessionManager();
	
	void Initialize();
	void Deinitialize();
	
	// Server Section
	
	// 클라이언트 관리 함수들
	/**
	 * 플레이어가 게임에 로그인할 때 호출되어 클라이언트 정보를 등록합니다.
	 * @param PlayerController 로그인한 플레이어 컨트롤러
	 */
	void RegisterClient(class APlayerController* PlayerController);

	/**
	 * 플레이어가 게임을 떠날 때 호출되어 클라이언트 정보를 제거합니다.
	 * @param ClientId 제거할 클라이언트 ID
	 */
	void UnregisterClient(int32 ClientId);
	
	/**
	* 등록된 모든 클라이언트 정보를 반환합니다.
	*/
	const TMap<int32, FStarLinkClientInfo>& GetAllClients() const { return ConnectedClients; }

	/**
	* 등록된 클라이언트 정보를 조회합니다.
	* @param ClientId 조회할 클라이언트 ID
	* @return 클라이언트 정보 (없으면 nullptr)
	*/
	FStarLinkClientInfo* GetClientInfo(int32 ClientId);
	
	/**
	 * Host Client로 새로운 Session을 생성합니다.
	 * @param HostClient Host로 설정할 클라이언트 정보
	 * @return 생성된 Session
	 */
	USLSession* CreateSession(int32 ClientId);


	void RemoveSession(int32 ClientId);

	/**
	 * Session ID로 Session을 조회합니다.
	 * @param SessionId 조회할 Session ID
	 * @return Session (없으면 nullptr)
	 */
	USLSession* GetSession(uint8 SessionId);

	/**
	 * 모든 Session을 반환합니다.
	 */
	const TMap<uint8, TObjectPtr<USLSession>>& GetAllSessions() const { return Sessions; }

protected:
	// 현재 연결된 클라이언트들의 정보를 저장
	UPROPERTY()
	TMap<int32, FStarLinkClientInfo> ConnectedClients;
	
	// Session ID -> Session 매핑
	UPROPERTY()
	TMap<uint8, TObjectPtr<USLSession>> Sessions;

	// Session ID 생성용 카운터
	uint8 SessionIdCounter = 199;
};
