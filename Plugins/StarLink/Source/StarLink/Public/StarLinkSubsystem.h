// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "StarLinkSubsystem.generated.h"

/**
 *
 */
UCLASS()
class STARLINK_API UStarLinkSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

	/**
	 * StarLinkSubsystem에 쉽게 접근할 수 있는 Static Getter
	 * @param WorldContextObject World Context를 제공하는 UObject (Actor, Component 등)
	 * @return StarLinkSubsystem 인스턴스 (없으면 nullptr)
	 */
	static UStarLinkSubsystem* Get(const UObject* WorldContextObject);

	class USLSessionManager* GetSessionManager() const { return SessionManager; };

	class USLService* GetSLService() const {return SLService;}

	class USLRoleManager* GetRoleManager() const { return RoleManager;}

private:
	FORCEINLINE bool HasAuthority() const { if (auto World = GetWorld()) { if (World->GetNetMode() == NM_DedicatedServer || World->GetNetMode() == NM_ListenServer) return true; } return false; }

	UPROPERTY()
	TObjectPtr<class USLRoleManager> RoleManager;

	UPROPERTY()
	TObjectPtr<class USLSessionManager> SessionManager;

	UPROPERTY()
	TObjectPtr<class USLService> SLService;
};
