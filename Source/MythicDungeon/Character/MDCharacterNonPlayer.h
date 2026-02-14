// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLRoleActor.h"
#include "Character/MDCharacter.h"
#include "MDCharacterNonPlayer.generated.h"

USTRUCT()
struct FRepMoveLite
{
	GENERATED_BODY()

	UPROPERTY() FVector_NetQuantize10 Pos;
	UPROPERTY() FVector_NetQuantize10 Vel;
	UPROPERTY() uint16 Yaw;
};

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API AMDCharacterNonPlayer : public AMDCharacter, public ISLRoleActor
{
	GENERATED_BODY()

public:
	AMDCharacterNonPlayer();

	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_ColorIndex)
	int32 ColorIndex = 0;

	UFUNCTION()
	void OnRep_ColorIndex();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void PossessedBy(AController* NewController) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Material")
	TArray<UMaterialInterface*> ColorMaterials;

	UPROPERTY(ReplicatedUsing=OnRep_MoveLite)
	FRepMoveLite RepMove;

	UFUNCTION()
	void OnRep_MoveLite();

	void ServerUpdateRepMove();

	virtual void Tick(float DeltaSeconds) override;

	// AcceptDistance 이하 이동은 리플리케이션 스킵
	UPROPERTY(EditDefaultsOnly, Category="Network")
	float AcceptDistanceSq = 10.f * 10.f;

	static uint16 CompressYaw(float InYaw) { return FRotator::CompressAxisToShort(InYaw); }
	static float DecompressYaw(uint16 InYaw) { return FRotator::DecompressAxisFromShort(InYaw); }

public:
	virtual void Master_OnAssign() override;
	virtual void Host_OnAssign() override;
	virtual void Master_OnReturn() override;
	virtual void Host_OnReturn() override;
};


