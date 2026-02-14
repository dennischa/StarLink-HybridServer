// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IpNetDriver.h"
#include "SLNetDriver.generated.h"

/**
 * 
 */
UCLASS()
class USLNetDriver : public UIpNetDriver
{
	GENERATED_BODY()

public:
	USLNetDriver(const FObjectInitializer& ObjectInitializer);
};