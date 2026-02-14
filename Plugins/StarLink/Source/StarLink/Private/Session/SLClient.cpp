// Fill out your copyright notice in the Description page of Project Settings.


#include "Session/SLClient.h"

#include "SLPlayerController.h"
#include "StarLink.h"
#include "StarLinkTypes.h"
#include "Net/DataChannel.h"
#include "Engine/NetDriver.h"
#include "Kismet/GameplayStatics.h"

USLClient::USLClient()
{
	ClientId = TEXT("ClientId");
}

void USLClient::Tick(float DeltaTime)
{
	if (NetDriver)
	{
		NetDriver->TickDispatch(DeltaTime);
		NetDriver->TickFlush(DeltaTime);
	}
}

bool USLClient::InitializeAsHost(FURL InURL, const FString& SessionId)
{
	URL = InURL;
	
	auto World = GetWorld();

	NetDriverName = FName(*FString::Printf(TEXT("SLNetDriver_Host_%s"), *SessionId));

	if (GEngine->CreateNamedNetDriver(World, NetDriverName, FName("SLNetDriver")))
	{
		NetDriver = GEngine->FindNamedNetDriver(World, NetDriverName);
	}

	NetDriver->GuidCache = GetWorld()->GetNetDriver()->GuidCache;

	if (NetDriver == nullptr)
	{
		return false;
	}
	
	NetDriver->SetWorld(World);
	NetDriver->Notify = this;
	
	FString Error;
	
	if (!NetDriver->InitListen(this, InURL, false, Error))
	{
		return false;
	}
	
	ClientRole = EStarLinkClientRole::Host;
	
	return true;
}

//  UPendingNetGame::InitNetDriver() 참고
bool USLClient::InitializeAsPeer(FURL HostURL, uint8 SessionId, SLClientInitializeComplete InInitializeCompleteDel)
{
	URL = HostURL;
	
	FString ConnectionError;

	auto World = GetWorld();

	NetDriverName = FName(*FString::Printf(TEXT("SLNetDriver_Peer_%d"), SessionId));

	if (GEngine->CreateNamedNetDriver(World, NetDriverName, FName("SLNetDriver")))
	{
		NetDriver = GEngine->FindNamedNetDriver(World, NetDriverName);
	}

	if (NetDriver == nullptr)
	{
		ConnectionError = TEXT("Error creating network driver.");
		return false;
	}
	
	NetDriver->GuidCache = GetWorld()->GetNetDriver()->GuidCache;
	
	if (!NetDriver->InitConnect(this, HostURL, ConnectionError))
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to InitConnect"));
		return false;
	}

	UNetConnection* ServerConn = NetDriver->ServerConnection;

	InitializeCompleteDel = InInitializeCompleteDel;
	
	// Kick off the connection handshake
	if (ServerConn->Handler.IsValid())
	{
		ServerConn->Handler->BeginHandshaking(
			FPacketHandlerHandshakeComplete::CreateUObject(this, &USLClient::SendInitialJoin));
	}
	else
	{
		SendInitialJoin();
	}
	
	return true;
}

EAcceptConnection::Type USLClient::NotifyAcceptingConnection()
{
	return EAcceptConnection::Accept;
}

void USLClient::NotifyAcceptedConnection(class UNetConnection* Connection)
{
	SL_LOG(LogStarLink, Log, TEXT("Accept Connection : %s"), *Connection->Describe());
}

bool USLClient::NotifyAcceptingChannel(class UChannel* Channel)
{
	SL_LOG(LogStarLink, Log, TEXT("UChannel : %s"), *Channel->Describe());

	return true;
}

void USLClient::NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, class FInBunch& Bunch)
{
	SL_LOG(LogStarLink, Log, TEXT("NotifyControlMessage Type: %d, Connection: %s"), MessageType, *Connection->Describe());

	// Peer (클라이언트)
	if (NetDriver->ServerConnection)
	{
		switch (MessageType)
		{
		case NMT_Challenge:
		{
			if (FNetControlMessage<NMT_Challenge>::Receive(Bunch, Connection->Challenge))
			{
				// Login
				FURL PartialURL(URL);
				PartialURL.Host = TEXT("");
				PartialURL.Port = PartialURL.UrlConfig.DefaultPort; 
				PartialURL.Map = TEXT("");
				
				ULocalPlayer* LocalPlayer = GEngine->GetFirstGamePlayer(GetWorld());
				if (LocalPlayer)
				{
					Connection->PlayerId = LocalPlayer->GetPreferredUniqueNetId();
				}
				
				FName OnlinePlatformName = NAME_None;
				
				Connection->ClientResponse = TEXT("0");
				FString URLString(PartialURL.ToString());
				FString OnlinePlatformNameString = OnlinePlatformName.ToString();

				NetDriver->SetWorld(GetWorld());
				NetDriver->Notify = this;

				FNetControlMessage<NMT_Login>::Send(Connection, Connection->ClientResponse, URLString, Connection->PlayerId, OnlinePlatformNameString);
				Connection->FlushNet();

				if (InitializeCompleteDel.IsBound())
				{
					InitializeCompleteDel.ExecuteIfBound(true, TEXT("Success"));
					InitializeCompleteDel.Unbind();
				}
				
				SL_LOG(LogStarLink, Log, TEXT("Challenge Received"));
			}
			else
			{
				Connection->Challenge.Empty();
				if (InitializeCompleteDel.IsBound())
				{
					InitializeCompleteDel.ExecuteIfBound(false, TEXT("Failed to Receive NMT_Challenge"));
					InitializeCompleteDel.Unbind();
				}
			}
					
			break;
		}
		case NMT_Failure:
		{
			FString ErrorMsg;

			if (FNetControlMessage<NMT_Failure>::Receive(Bunch, ErrorMsg))
			{
				if (ErrorMsg.IsEmpty())
				{
					ErrorMsg = NSLOCTEXT("NetworkErrors", "GenericConnectionFailed", "Connection Failed.").ToString();
				}

				if (Connection != nullptr)
				{
					Connection->SendCloseReason(ENetCloseResult::FailureReceived);
				}
				
				if (Connection != nullptr)
				{
					Connection->Close(ENetCloseResult::FailureReceived);
				}
			}

			break;
		}
		}
	}
	// Host (서버)
	else
	{
		switch (MessageType)
		{
		case NMT_Hello:
		{
				Connection->SendChallengeControlMessage();
				break;
		}
		case NMT_Login:
		{
				FString ErrorMsg;
				
				// Admit or deny the player here.
				FUniqueNetIdRepl UniqueIdRepl;
				FString OnlinePlatformName;
				FString& RequestURL = Connection->RequestURL;

				// Expand the maximum string serialization size, to accommodate extremely large Fortnite join URL's.
				Bunch.ArMaxSerializeSize += (16 * 1024 * 1024);

				bool bReceived = FNetControlMessage<NMT_Login>::Receive(Bunch, Connection->ClientResponse, RequestURL, UniqueIdRepl,
																		OnlinePlatformName);
				if (bReceived)
				{
					// Only the options/portal for the URL should be used during join
					const TCHAR* NewRequestURL = *RequestURL;

					for (; *NewRequestURL != '\0' && *NewRequestURL != '?' && *NewRequestURL != '#'; NewRequestURL++){}
					
					UE_LOG(LogStarLink, Log, TEXT("Login request: %s userId: %s platform: %s"), NewRequestURL, UniqueIdRepl.IsValid() ? *UniqueIdRepl->ToString() : TEXT("UNKNOWN"), *OnlinePlatformName);

					Connection->PlayerId = UniqueIdRepl;

					// Spawn the player-controller for this network player.
					
					Connection->PlayerController = SpawnPlayerController(uint8(Connection->Children.Num()));

					if (Connection->PlayerController == nullptr)
					{
						ErrorMsg = TEXT("Failed to SpawnPlayerController");
						FNetControlMessage<NMT_Failure>::Send(Connection, ErrorMsg);
						Connection->FlushNet(true);
					}
					
					Connection->ClientLoginState = EClientLoginState::ReceivedJoin;
					Connection->OwningActor = Connection->PlayerController;

					Connection->SetClientWorldPackageName(GetWorld()->PersistentLevel->GetOutermost()->GetFName());
				}
				else
				{
					ErrorMsg = TEXT("Failed to Receive NMT_Login");
					FNetControlMessage<NMT_Failure>::Send(Connection, ErrorMsg);
					Connection->FlushNet(true);
				}
				
				break;
		}
		}
	}
}

void USLClient::SendInitialJoin()
{
	SL_LOG(LogStarLink, Log, TEXT("SendInitialJoin"));
	if (NetDriver != nullptr)
	{
		UNetConnection* ServerConn = NetDriver->ServerConnection;

		if (ServerConn != nullptr)
		{
			FString EncryptionToken = TEXT("");
			uint8 IsLittleEndian = uint8(PLATFORM_LITTLE_ENDIAN);
			uint32 LocalNetworkVersion = FNetworkVersion::GetLocalNetworkVersion();
			EEngineNetworkRuntimeFeatures LocalNetworkFeatures = NetDriver->GetNetworkRuntimeFeatures();
			
			FNetControlMessage<NMT_Hello>::Send(ServerConn, IsLittleEndian, LocalNetworkVersion, EncryptionToken, LocalNetworkFeatures);

			ServerConn->FlushNet();
		}
	}
}

APlayerController* USLClient::SpawnPlayerController(uint8 PlayerIndex)
{
	static uint8 Index = 1;
	FActorSpawnParameters SpawnInfo;
	//SpawnInfo.Instigator = this;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save player controllers into a map
	SpawnInfo.bDeferConstruction = true;
	SpawnInfo.Name = FName(FString::Printf(TEXT("%s_PlayerController_%d"), *NetDriverName.ToString(), Index++));

	APlayerController* NewPC = GetWorld()->SpawnActor<APlayerController>(ASLPlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewPC)
	{
		UGameplayStatics::FinishSpawningActor(NewPC, FTransform(FRotator::ZeroRotator, FVector::ZeroVector));

		NewPC->NetPlayerIndex = PlayerIndex;
		NewPC->SetRole(ROLE_Authority);
		NewPC->SetAutonomousProxy(true);
		// 일단 임시
		NewPC->SetViewTarget(NewPC);

		NewPC->SetNetDriverName(NetDriverName);

		SL_NETWORK_LOG(NewPC, LogStarLink, Log, TEXT("New Player Controller Success : %d"), PlayerIndex);
	}
	
	return NewPC;
}
