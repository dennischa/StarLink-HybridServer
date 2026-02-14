// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MDCharacterNonPlayer.h"

#include "MythicDungeon.h"
#include "StarLink.h"
#include "AI/MDAIController.h"
#include "Engine/PackageMapClient.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/StarLinkPlayerController.h"

AMDCharacterNonPlayer::AMDCharacterNonPlayer()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->PrimaryComponentTick.TickInterval = 0.1f;

	PrimaryActorTick.bCanEverTick = true;
}

void AMDCharacterNonPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (auto NetDriver = GetNetDriver())
	{
		FNetworkGUID GUID = NetDriver->GuidCache->GetOrAssignNetGUID(this);

		MD_NETWORK_LOG(LogMD, Log, TEXT("GUID: %d"), GUID.Value);
	}

	// 기본 MovementReplication 비활성화 - RepMove로 대체
	SetReplicateMovement(false);

	USkeletalMeshComponent* MeshComp = GetMesh();

	if (GetNetMode() == NM_DedicatedServer)
	{
		if (MeshComp)
		{
			MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
			MeshComp->bComponentUseFixedSkelBounds = true;
		}
	}
	// 클라이언트
	else if (!HasAuthority())
	{
		// CMC의 SimulatedTick 비활성화 
		GetCharacterMovement()->SetComponentTickEnabled(false);

		// SkeletalMesh 최적화
		if (MeshComp)
		{
			MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			MeshComp->bEnableUpdateRateOptimizations = true;
			MeshComp->bComponentUseFixedSkelBounds = true;
		}
	}
}

void AMDCharacterNonPlayer::OnRep_MoveLite()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->Velocity = FVector(RepMove.Vel);
	}
}

void AMDCharacterNonPlayer::ServerUpdateRepMove()
{
	if (!HasAuthority()) return;

	RepMove.Pos = GetActorLocation();
	RepMove.Vel = GetVelocity();
	RepMove.Yaw = CompressYaw(GetActorRotation().Yaw);

	ForceNetUpdate();
}

void AMDCharacterNonPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		// Authority: AcceptDistance 초과 시 RepMove 갱신
		const float DistSq = FVector::DistSquared(GetActorLocation(), FVector(RepMove.Pos));
		if (DistSq > AcceptDistanceSq || GetVelocity().IsNearlyZero())
		{
			ServerUpdateRepMove();
		}
		return;
	}

	// 클라이언트: 위치 보간
	const FVector Cur = GetActorLocation();
	const FVector Target = FVector(RepMove.Pos);
	const FVector NewPos = FMath::VInterpTo(Cur, Target, DeltaSeconds, 5.f);

	SetActorLocation(NewPos, false);
	SetActorRotation(FRotator(0, DecompressYaw(RepMove.Yaw), 0));
	
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->Velocity = FVector(RepMove.Vel);
	}
}

void AMDCharacterNonPlayer::Master_OnAssign()
{
	auto* MDAIController = Cast<AMDAIController>(GetController());

	if (MDAIController)
	{
		MDAIController->StopAI();
	}
	
	GetCharacterMovement()->SetComponentTickEnabled(false);

	SetReplicates(false);
}

void AMDCharacterNonPlayer::Host_OnAssign()
{
	if (HasAuthority())
	{
		GetCharacterMovement()->SetComponentTickEnabled(true);

		auto* MDAIController = Cast<AMDAIController>(GetController());

		if (MDAIController)
		{
			MDAIController->SetRole(ROLE_Authority);
			MDAIController->Possess(this);
		}

		SetReplicates(true);

		ServerUpdateRepMove();
	}
}

void AMDCharacterNonPlayer::Master_OnReturn()
{
	if (HasAuthority())
	{
		GetCharacterMovement()->SetComponentTickEnabled(true);

		SetReplicates(true);

		auto* MDAIController = Cast<AMDAIController>(GetController());

		if (MDAIController)
		{
			MDAIController->SetRole(ROLE_Authority);
			MDAIController->Possess(this);
		}

		ServerUpdateRepMove();
	}
}

void AMDCharacterNonPlayer::Host_OnReturn()
{
	auto* MDAIController = Cast<AMDAIController>(GetController());

	if (MDAIController)
	{
		MDAIController->StopAI();
	}

	GetCharacterMovement()->SetComponentTickEnabled(false);

	SetReplicates(false);
}

void AMDCharacterNonPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AMDCharacterNonPlayer, ColorIndex, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AMDCharacterNonPlayer, RepMove, COND_None, REPNOTIFY_Always);
}

void AMDCharacterNonPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (GetNetMode() == NM_DedicatedServer)
		{
			ColorIndex = 0;
		}
		else
		{
			AStarLinkPlayerController* SLPlayer = Cast<AStarLinkPlayerController>(GetWorld()->GetFirstPlayerController());

			if (SLPlayer)
			{
				ColorIndex = SLPlayer->GetClientId() % 3 + 1;
			}
		}

		OnRep_ColorIndex();
		ServerUpdateRepMove();
		ForceNetUpdate();
	}
}

void AMDCharacterNonPlayer::OnRep_ColorIndex()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		MeshComp->SetMaterial(0, ColorMaterials[ColorIndex]);
	}
}
