// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MDGameMode.generated.h"

/**
 *
 */
UCLASS()
class MYTHICDUNGEON_API AMDGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMDGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void BeginPlay() override;
	void SpawnNPCs();

	// 플레이어 로그인/로그아웃 처리
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category=NPC)
	TSubclassOf<class ACharacter> NPCClass;

	UPROPERTY(EditAnywhere, Category=NPC)
	int32 NumNPCs = 5;

	bool GetRandomPointOnNav(FVector& OutLoc) const;

};
