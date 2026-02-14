// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StarLinkTypes.generated.h"

/**
 * StarLink 클라이언트의 역할을 정의
 * Session 내에서의 역할 (전통적인 Server-Client Role과 구분)
 */
UENUM(BlueprintType)
enum class EStarLinkClientRole : uint8
{
	None,      // 역할 미할당
	Host,      // HostClient - Session을 소유하고 관리
	Peer       // Session에 연결된 참가자
};

USTRUCT(BlueprintType)
struct STARLINK_API FStarLinkClientInfo
{
	GENERATED_BODY()

	int32 ClientId;

	FURL Url;

	EStarLinkClientRole Role;

	bool bIsReady;

	TWeakObjectPtr<class USLRPCHelperComponent> RPCHelper;

	FStarLinkClientInfo()
		: Role(EStarLinkClientRole::None)
		, bIsReady(false)
	{
	}
};

USTRUCT(BlueprintType)
struct STARLINK_API FStarLinkSessionInfo
{
	GENERATED_BODY()

	FString SessionId;

	FString HostClientId;

	TArray<FStarLinkClientInfo> Clients;

	bool bIsActive;

	FStarLinkSessionInfo()
		: bIsActive(false)
	{
	}
};