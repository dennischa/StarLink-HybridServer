// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SLRoleActor.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USLRoleActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STARLINK_API ISLRoleActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Master_OnAssign() = 0;
	virtual void Host_OnAssign() = 0;
	
	virtual void Master_OnReturn() = 0;
	virtual void Host_OnReturn() = 0;
};
