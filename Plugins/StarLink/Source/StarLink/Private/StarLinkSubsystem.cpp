// Fill out your copyright notice in the Description page of Project Settings.


#include "StarLinkSubsystem.h"

#include "StarLink.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/NetConnection.h"
#include "Kismet/GameplayStatics.h"
#include "Service/SLService.h"
#include "Manager/SLRoleManager.h"
#include "Manager/SLSessionManager.h"

void UStarLinkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GetWorld())
	{
		auto NetMode = GetWorld()->GetNetMode();
		SL_LOG(LogStarLink, Log, TEXT("%s"), *StarLinkLogHelper::GetNetModeString(NetMode));
	}

	SLService = NewObject<USLService>(this);
	
	if (HasAuthority())
	{
		SessionManager = NewObject<USLSessionManager>(this);
		SessionManager->Initialize();

		RoleManager = NewObject<USLRoleManager>(this);

		SL_LOG(LogStarLink, Log, TEXT("StarLinkSubsystem Initialize on SERVER"));
	}
	else
	{
		SL_LOG(LogStarLink, Log, TEXT("StarLinkSubsystem Initialize on CLIENT"));
	}
}

UStarLinkSubsystem* UStarLinkSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UStarLinkSubsystem>();
}

void UStarLinkSubsystem::Deinitialize()
{
	Super::Deinitialize();
	SL_LOG(LogStarLink, Log, TEXT("Begin"));

	if (SessionManager)
	{
		SessionManager->Deinitialize();
	}
}

void UStarLinkSubsystem::Tick(float DeltaTime)
{
	if (SLService)
	{
		SLService->Tick(DeltaTime);
	}
}

TStatId UStarLinkSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UStarLinkSubsystem, STATGROUP_Tickables);
}

bool UStarLinkSubsystem::IsTickable() const
{
	return !IsTemplate() && GetGameInstance() != nullptr;
}

UWorld* UStarLinkSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}
