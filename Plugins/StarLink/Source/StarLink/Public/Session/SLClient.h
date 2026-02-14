// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StarLinkTypes.h"
#include "SLClient.generated.h"

DECLARE_DELEGATE_TwoParams(SLClientInitializeComplete, bool, FString);

/**
 * 
 */
UCLASS()
class STARLINK_API USLClient : public UObject, public FNetworkNotify
{
	GENERATED_BODY()

public:
	USLClient();

	bool InitializeAsHost(FURL InURL, const FString& SessionId);
	bool InitializeAsPeer(FURL HostURL, uint8 SessionId, SLClientInitializeComplete InitializeCompleteDel);

	void Tick(float DeltaTime);

	FName GetNetDriverName() const { return NetDriverName;}

	UNetDriver* GetNetDriver() const { return NetDriver;}
	
	// Peer Client FNetworkNotify

	virtual EAcceptConnection::Type NotifyAcceptingConnection() override;
	
	virtual void NotifyAcceptedConnection(class UNetConnection* Connection) override;

	virtual bool NotifyAcceptingChannel(class UChannel* Channel) override;

	virtual void NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, class FInBunch& Bunch) override;

protected:
	UFUNCTION()
	void SendInitialJoin();

	APlayerController* SpawnPlayerController(uint8 PlayerIndex);
private:
	EStarLinkClientRole ClientRole;

	TObjectPtr<class UNetDriver> NetDriver;

	FString ClientId;

	FName NetDriverName;

	SLClientInitializeComplete InitializeCompleteDel;

	FURL URL;
};
