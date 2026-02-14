// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLService.generated.h"

DECLARE_DELEGATE_OneParam(FOnNetworkFuncFinished, const FSLRPCResponseData&);

/**
 * 
 */
UCLASS()
class STARLINK_API USLService : public UObject
{
	GENERATED_BODY()
public:
	USLService();

	void Initialize();

	void Tick(float DeltaTime);

	FSLRPCResponseData CreateHostClient(const FString& SessionId, const FURL& InURL);

	TSharedPtr<FSLRPCResponse> CreatePeerClient(uint8 SessionId, const FURL& InURL);
	
	FSLRPCResponseData Assign(uint32 GUID);

	FSLRPCResponseData Return(uint32 GUID);

	TWeakObjectPtr<USLClient> GetPeerClient(uint8 SessionId);


protected:
	FString HostSessionId;
	
	UPROPERTY()
	TObjectPtr<USLClient> HostClient;

	UPROPERTY()
	TMap<uint8, TObjectPtr<USLClient>> PeerClients;
	
	//TSharedPtr<FSLRPCResponse> Response;
};
