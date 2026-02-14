// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLRoleManager.generated.h"

USTRUCT()
struct FSLAssignActorInfo
{
	GENERATED_BODY()

	static FSLAssignActorInfo DefaultInfo;

	FSLAssignActorInfo() : GUID(0), Actor(nullptr) {}
	FSLAssignActorInfo(uint32 InGUID, AActor* InActor) : GUID(InGUID), Actor(InActor) {}

	uint32 GUID = 1;

	UPROPERTY()
	TWeakObjectPtr<AActor> Actor = nullptr;

	bool IsDefault() const { return GUID == 1;}
};

USTRUCT()
struct FSLRoleAssignment
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSLAssignActorInfo> Infos;

	TSet<uint32> GetGUIDsSet() const
	{
		TSet<uint32> Result;
		for (const FSLAssignActorInfo& Info : Infos)
		{
			Result.Add(Info.GUID);
		}
		return Result;
	}
};

/**
 *
 */
UCLASS()
class STARLINK_API USLRoleManager : public UObject
{
	GENERATED_BODY()

public:
	void Assign(AActor* Actor, uint8 SessionId);

	void Assign(uint32 GUID, uint8 SessionId);

	void Return(AActor* Actor);
	
	void Return(uint32 GUID);

	void OnHandOver(uint8 SessionId);

	bool IsAssigned(uint32 GUID) const { return AssignedGUIDSet.Contains(GUID); }

protected:
	UPROPERTY()
	TMap<uint8, FSLRoleAssignment> FSLRoleAssignments;

	TSet<uint32> AssignedGUIDSet;

	void InternalAssignImplementation(AActor* Actor, uint8 SessionId, FNetworkGUID GUID = FNetworkGUID::GetDefault());

	void InternalReturnImplementation(uint8 SessionId, FSLAssignActorInfo Info);

	const FSLAssignActorInfo& GetAssignActorInfo(uint32 GUID, uint8& OutSessionId);
	const FSLAssignActorInfo& GetAssignActorInfo(AActor* Actor, uint8& OutSessionId);
};

