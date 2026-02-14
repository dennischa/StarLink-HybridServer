// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "NiagaraSystem.h"
#include "StarLinkPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "MDPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MYTHICDUNGEON_API AMDPlayerController : public AStarLinkPlayerController, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMDPlayerController();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }

	virtual void OnRep_PlayerState() override;

	virtual void OnPossess(APawn* InPawn) override;
	
	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SetDestinationClickAction;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* AttackAction;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SetDestinationTouchAction;

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupInputComponent() override;
	
	// To add mapping context
	virtual void BeginPlay();

	/** Input handlers for SetDestination action. */
	void OnClickStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();

	// GAS Input
	void GASInputTriggered(int32 InputId);

	// Check if character can handle movement input (not attacking, stunned, etc.)
	bool CanHandleMoveInput() const;

private:
	FVector CachedDestination;

	bool bIsTouch; // Is it a touch device
	float FollowTime; // For how long it has been pressed

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MythicDungeon|Abilities")
	TObjectPtr<class UAbilitySystemComponent> ASC;
};