// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define SL_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// 범용 네트워크 로그 - Actor 포인터를 받아서 해당 Actor의 네트워크 정보 출력
// 사용 예: MD_NETWORK_ETCLOG(GetOwner(), LogMD, Warning, TEXT("Message"))
// 사용 예: MD_NETWORK_ETCLOG(GetOwningActor(), LogMD, Warning, TEXT("Message"))
#define SL_NETWORK_LOG(Actor, LogCat, Verbosity, Format, ...) \
if (Actor) \
{ \
const ENetMode NetMode = (Actor)->GetNetMode(); \
const FString NetModeStr = (NetMode == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((NetMode == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")); \
const FString LocalRoleStr = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), (Actor)->GetLocalRole()); \
const FString RemoteRoleStr = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), (Actor)->GetRemoteRole()); \
UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), *NetModeStr, *LocalRoleStr, *RemoteRoleStr, ANSI_TO_TCHAR(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
}


namespace StarLinkLogHelper
{
	// ENetMode를 읽기 쉬운 문자열로 변환하는 Helper 함수
	static FString GetNetModeString(ENetMode NetMode)
	{
		switch (NetMode)
		{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			return FString::Printf(TEXT("Client%d"), GPlayInEditorID);
		default:
			return TEXT("Unknown");
		}
	}

	// Actor의 NetMode를 문자열로 변환하는 Helper 함수
	static FString GetNetModeString(const AActor* Actor)
	{
		if (!Actor)
		{
			return TEXT("InvalidObject");
		}
		return GetNetModeString(Actor->GetNetMode());
	}
}

DECLARE_LOG_CATEGORY_EXTERN(LogStarLink, Log, All);

class FStarLinkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};