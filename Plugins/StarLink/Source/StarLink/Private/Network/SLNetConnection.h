// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"
#include "SLNetConnection.generated.h"

/**
 * StarLink 전용 NetConnection
 * 하이브리드 데디 서버를 위한 커스텀 연결 클래스
 */
UCLASS()
class STARLINK_API USLNetConnection : public UIpConnection
{
	GENERATED_BODY()

public:
	USLNetConnection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void HandleClientPlayer(APlayerController* PC, UNetConnection* NetConnection) override;
};
