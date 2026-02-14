// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SLRoleManager.h"

#include "SLRoleActor.h"
#include "StarLink.h"
#include "StarLinkSubsystem.h"
#include "Engine/PackageMapClient.h"
#include "Manager/SLSessionManager.h"
#include "Session/SLSession.h"

FSLAssignActorInfo FSLAssignActorInfo::DefaultInfo;

void USLRoleManager::Assign(AActor* Actor, uint8 SessionId)
{
	if (auto NetDriver = GetWorld()->GetNetDriver())
	{
		FNetworkGUID GUID = NetDriver->GuidCache->GetOrAssignNetGUID(Actor);

		InternalAssignImplementation(Actor, SessionId, GUID);
	}
}

void USLRoleManager::Assign(uint32 GUID, uint8 SessionId)
{
	if (auto NetDriver = GetWorld()->GetNetDriver())
	{
		AActor* Actor = Cast<AActor>(NetDriver->GuidCache->GetObjectFromNetGUID(GUID, true));

		InternalAssignImplementation(Actor, SessionId, GUID);
	}
}

void USLRoleManager::Return(AActor* Actor)
{
	if (Actor == nullptr)
	{
		SL_LOG(LogStarLink, Error, TEXT("Actor is null"));
		return;
	}
	
	uint8 SessionId;
	auto Info = GetAssignActorInfo(Actor, SessionId);

	if (Info.IsDefault())
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to Find ActorInfo, Actor: %s"), *Actor->GetName());
		return;
	}
	
	InternalReturnImplementation(SessionId, Info);
}

void USLRoleManager::Return(uint32 GUID)
{
	uint8 SessionId;
	auto Info = GetAssignActorInfo(GUID, SessionId);

	if (Info.IsDefault())
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to Find ActorInfo, GUID: %d"), GUID);
		return;
	}

	InternalReturnImplementation(SessionId, Info);
}

void USLRoleManager::OnHandOver(uint8 SessionId)
{
	if (FSLRoleAssignment* RoleAssignment = FSLRoleAssignments.Find(SessionId))
	{
		auto SLSessionManager = UStarLinkSubsystem::Get(this)->GetSessionManager();

		auto* Session = SLSessionManager->GetSession(SessionId);

		for (auto Info : RoleAssignment->Infos)
		{
			if (Info.Actor.IsValid())
			{
				Session->Master_OnReturn(Info.Actor.Get());
				AssignedGUIDSet.Remove(Info.GUID);
			}
		}

		FSLRoleAssignments.Remove(SessionId);
	}
	else
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to Find SessionId: %d"), SessionId);
	}
}

const FSLAssignActorInfo& USLRoleManager::GetAssignActorInfo(uint32 GUID, uint8& OutSessionId )
{
	for (const auto& FSLRoleAssignment : FSLRoleAssignments)
	{
		for (const auto& Info: FSLRoleAssignment.Value.Infos)
		{
			if (Info.GUID == GUID)
			{
				OutSessionId = FSLRoleAssignment.Key;
				return Info;
			}
		}
	}

	return FSLAssignActorInfo::DefaultInfo;
}

const FSLAssignActorInfo& USLRoleManager::GetAssignActorInfo(AActor* InActor, uint8& OutSessionId)
{
	if (InActor)
	{
		for (const auto& FSLRoleAssignment : FSLRoleAssignments)
		{
			for (const auto& Info : FSLRoleAssignment.Value.Infos)
			{
				if (Info.Actor == InActor)
				{
					OutSessionId = FSLRoleAssignment.Key;
					return Info;
				}
			}
		}
	}
	
	return FSLAssignActorInfo::DefaultInfo;
}

void USLRoleManager::InternalAssignImplementation(AActor* Actor, uint8 SessionId, FNetworkGUID GUID)
{
	if (Actor == nullptr)
	{
		SL_LOG(LogStarLink, Error, TEXT("Actor is null"));
		return;
	}
	
	if (AssignedGUIDSet.Contains(GUID.Value))
	{
		SL_LOG(LogStarLink, Error, TEXT("Already Assigned GUID : %d"), GUID.Value)
		return;
	}
	
	if (!FSLRoleAssignments.Find(SessionId))
	{
		FSLRoleAssignments.Add(SessionId, FSLRoleAssignment());
	}

	if (GUID.IsDefault())
	{
		if (auto NetDriver = GetWorld()->GetNetDriver())
		{
			GUID = NetDriver->GuidCache->GetOrAssignNetGUID(Actor);
		}

		if (GUID.IsDefault())
		{
			SL_LOG(LogStarLink, Error, TEXT("Failed to Find GUID: %s"), *Actor->GetName());
			return;
		}
	}

	auto SLSessionManager = UStarLinkSubsystem::Get(this)->GetSessionManager();

	auto* Session = SLSessionManager->GetSession(SessionId);

	if (Session == nullptr)
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to Find Session Id: %d"), SessionId);
		return;
	}

	Session->Assign(Actor);

	FSLAssignActorInfo AssignActorInfo(GUID.Value, Actor);
	FSLRoleAssignments[SessionId].Infos.Add(AssignActorInfo);

	AssignedGUIDSet.Add(GUID.Value);
}

void USLRoleManager::InternalReturnImplementation(uint8 SessionId, FSLAssignActorInfo Info)
{
	if (!AssignedGUIDSet.Contains(Info.GUID))
	{
		SL_LOG(LogStarLink, Error, TEXT(" GUID %d Is Not Assigned yet"), Info.GUID)
		return;
	}
	
	auto SLSessionManager = UStarLinkSubsystem::Get(this)->GetSessionManager();

	auto* Session = SLSessionManager->GetSession(SessionId);

	if (Session == nullptr)
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to Find Session Id: %d"), SessionId);
		return;
	}

	if (!Info.Actor.Get())
	{
		SL_LOG(LogStarLink, Error, TEXT("Failed to Find Actor"));
	}

	Session->Return(Info.Actor.Get());

	AssignedGUIDSet.Remove(Info.GUID);
	
	FSLRoleAssignments[SessionId].Infos.RemoveAll([&Info](const FSLAssignActorInfo& AssignInfo)
	{
		return AssignInfo.GUID == Info.GUID;
	});
}

