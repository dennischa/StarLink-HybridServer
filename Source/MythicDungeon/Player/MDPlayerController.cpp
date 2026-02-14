// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MDPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MDPlayerState.h"
#include "MythicDungeon.h"
#include "NiagaraFunctionLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Tags/MDGameplayTag.h"

AMDPlayerController::AMDPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void AMDPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AMDPlayerState* PS = GetPlayerState<AMDPlayerState>())
	{
		ASC = PS->GetAbilitySystemComponent();
	}
}

void AMDPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Server
	if (HasAuthority())
	{
		if (AMDPlayerState* PS = GetPlayerState<AMDPlayerState>())
		{
			ASC = PS->GetAbilitySystemComponent();
		}
	}
}

void AMDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AMDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AMDPlayerController::OnClickStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AMDPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AMDPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AMDPlayerController::OnSetDestinationReleased);
		
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AMDPlayerController::GASInputTriggered, 0);
	}
}


void AMDPlayerController::OnClickStarted()
{
	StopMovement();
}

void AMDPlayerController::OnSetDestinationTriggered()
{
	// Block input if character cannot handle movement (e.g., attacking, stunned)
	if (!CanHandleMoveInput())
	{
		return;
	}

	FollowTime += GetWorld()->GetDeltaSeconds();

	FHitResult Hit;
	bool bHitSucceeded = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	if (bHitSucceeded)
	{
		CachedDestination = Hit.Location;
	}

	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AMDPlayerController::OnSetDestinationReleased()
{
	// Block input if character cannot handle movement (e.g., attacking, stunned)
	if (!CanHandleMoveInput())
	{
		FollowTime = 0.f;
		return;
	}

	if (FollowTime <= ShortPressThreshold)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

void AMDPlayerController::GASInputTriggered(int32 InputId)
{
	if (!ASC)
	{
		return;
	}

	MD_NETWORK_LOG(LogMDNetwork, Log, TEXT("GASInputTrigger: %d"), InputId);

	auto* Spec = ASC->FindAbilitySpecFromInputID(InputId);

	if (Spec)
	{
		Spec->InputPressed = true;
		if (Spec->IsActive())
		{
			ASC->AbilitySpecInputPressed(*Spec);
		}
		else
		{
			ASC->TryActivateAbility(Spec->Handle  );
		}
	}
}

bool AMDPlayerController::CanHandleMoveInput() const
{
	if (!ASC)
	{
		return true;
	}
	
	if (ASC->HasMatchingGameplayTag(ABTAG_CHARACTER_ISATTACKING))
	{
		return false;
	}

	return true;
}
