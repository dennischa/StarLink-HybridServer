// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/SLNetConnection.h"

#include "StarLink.h"

USLNetConnection::USLNetConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USLNetConnection::HandleClientPlayer(APlayerController* PC, UNetConnection* NetConnection)
{
	// 하이브리드 데디 서버: LocalPlayer 연결 스킵
	// 데디 서버는 LocalPlayer가 없으므로 연결하지 않음
	// Super::HandleClientPlayer()를 호출하면 LocalPlayer를 찾으려 하므로 호출하지 않음

	SL_LOG(LogStarLink, Log, TEXT("HandleClientPlayer - PC: %s, Connection: %s"),
		*GetNameSafe(PC),
		*GetName());

	check(Driver->GetWorld());

	// Init the new playerpawn.
	PC->SetRole(ROLE_AutonomousProxy);
	PC->NetConnection = NetConnection;
	
	LastReceiveTime = Driver->GetElapsedTime();
	SetConnectionState(EConnectionState::USOCK_Open);
	PlayerController = PC;
	OwningActor = PC;

	NotifyConnectionUpdated();
}
