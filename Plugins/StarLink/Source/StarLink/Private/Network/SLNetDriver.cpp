// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/SLNetDriver.h"

#include "SLNetConnection.h"
#include "StarLink.h"
#include "Engine/NetworkObjectList.h"


USLNetDriver::USLNetDriver(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DebugRelevantActors = true;

	NetConnectionClass = USLNetConnection::StaticClass();
}

