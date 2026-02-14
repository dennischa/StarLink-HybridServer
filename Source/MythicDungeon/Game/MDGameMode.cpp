// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MDGameMode.h"

#include "EngineUtils.h"
#include "MythicDungeon.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Manager/SLSessionManager.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "StarLink/Public/StarLinkSubsystem.h"

AMDGameMode::AMDGameMode()
{
}

void AMDGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (!FParse::Value(FCommandLine::Get(), TEXT("numnpcs="), NumNPCs))
	{
		NumNPCs = 5;
	}
}

void AMDGameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnNPCs();
}

void AMDGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (UStarLinkSubsystem* StarLinkSubsystem = UStarLinkSubsystem::Get(this))
	{
		if (USLSessionManager* SessionManager = StarLinkSubsystem->GetSessionManager())
		{
			if (APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>())
			{
				MD_NETWORK_ETCLOG(NewPlayer, LogMDNetwork, Log, TEXT("Player ID : %d"), PlayerState->PlayerId);
				SessionManager->RegisterClient(NewPlayer);
			}
		}
	}
}

void AMDGameMode::Logout(AController* Exiting)
{
	// StarLink Subsystem에서 클라이언트 제거
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		if (UStarLinkSubsystem* StarLinkSubsystem = UStarLinkSubsystem::Get(this))
		{
			if (USLSessionManager* SessionManager = StarLinkSubsystem->GetSessionManager())
			{
				if (APlayerState* PlayerState = PC->GetPlayerState<APlayerState>())
				{
					SessionManager->UnregisterClient(PlayerState->GetPlayerId());

					SessionManager->RemoveSession(PlayerState->GetPlayerId());
				}
			}
		}
	}

	Super::Logout(Exiting);
}

void AMDGameMode::SpawnNPCs()
{
	if (!ensure(NPCClass))
		return;

	const FVector Origin = FVector::ZeroVector;

	for (int32 i=0; i < NumNPCs; i++)
	{
		FVector SpawnLoc;
		if (GetRandomPointOnNav(SpawnLoc))
		{
			const FRotator SpawnRot = FRotator::ZeroRotator;
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			auto* NPC = GetWorld()->SpawnActor<ACharacter>(NPCClass, SpawnLoc, SpawnRot, Params);
		}
		
	}
}

bool AMDGameMode::GetRandomPointOnNav(FVector& OutLoc) const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return false;
	
	for (auto Vol : TActorRange<ANavMeshBoundsVolume>(GetWorld()))
	{
		const FBox Box = Vol->GetComponentsBoundingBox(true);
		const FVector Center = Box.GetCenter();
		const FVector Extent = Box.GetExtent();

		const FVector Candidate = UKismetMathLibrary::RandomPointInBoundingBox(Center, Extent);

		FNavLocation Projected;
		const FVector QueryExtent(300.f, 300.f, 1000.f);

		if (NavSys->ProjectPointToNavigation(Candidate, Projected, QueryExtent))
		{
			OutLoc = Projected.Location;
			return true;
		}
	}

	return false;
}
