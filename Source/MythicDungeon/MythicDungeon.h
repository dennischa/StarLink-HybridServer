// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMythicDungeon, Log, All);

// Actor 기반 네트워크 로그 (현재 객체가 Actor인 경우)
#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_SUBLOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetLocalRole()))
#define LOG_SUBREMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetRemoteRole()))
#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define MD_NETWORK_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define MD_NETWORK_SUBLOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_SUBLOCALROLEINFO, LOG_SUBREMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// GameplayAbility용 네트워크 로그 매크로
#define LOG_GA_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetAvatarActorFromActorInfo()->GetLocalRole()))
#define LOG_GA_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetAvatarActorFromActorInfo()->GetRemoteRole()))
#define LOG_GA_NETMODEINFO ((GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define MD_GA_NETWORK_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_GA_NETMODEINFO, LOG_GA_LOCALROLEINFO, LOG_GA_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

// 범용 네트워크 로그 - Actor 포인터를 받아서 해당 Actor의 네트워크 정보 출력
// 사용 예: MD_NETWORK_ETCLOG(GetOwner(), LogMD, Warning, TEXT("Message"))
// 사용 예: MD_NETWORK_ETCLOG(GetOwningActor(), LogMD, Warning, TEXT("Message"))
#define MD_NETWORK_ETCLOG(Actor, LogCat, Verbosity, Format, ...) \
	if (Actor) \
	{ \
		const ENetMode NetMode = (Actor)->GetNetMode(); \
		const FString NetModeStr = (NetMode == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((NetMode == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")); \
		const FString LocalRoleStr = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), (Actor)->GetLocalRole()); \
		const FString RemoteRoleStr = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), (Actor)->GetRemoteRole()); \
		UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), *NetModeStr, *LocalRoleStr, *RemoteRoleStr, ANSI_TO_TCHAR(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
	}

// NetMode를 FString으로 변환하는 유틸리티 함수
inline FString NetModeToString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_Standalone:
		return TEXT("STANDALONE");
	case NM_DedicatedServer:
		return TEXT("DEDICATED_SERVER");
	case NM_ListenServer:
		return TEXT("LISTEN_SERVER");
	case NM_Client:
		return FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID);
	default:
		return TEXT("UNKNOWN");
	}
}

// Actor의 NetMode를 FString으로 변환하는 유틸리티 함수
inline FString GetNetModeAsString(const AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("INVALID_ACTOR");
	}
	return NetModeToString(Actor->GetNetMode());
}

DECLARE_LOG_CATEGORY_EXTERN(LogMDNetwork, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMD, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMDAnim, Log, All);
