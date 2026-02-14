// Fill out your copyright notice in the Description page of Project Settings.


#include "SLPlayerController.h"

#include "StarLink.h"

void ASLPlayerController::OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection)
{
	SL_NETWORK_LOG(this, LogStarLink, Log, TEXT("Begin"));
}
