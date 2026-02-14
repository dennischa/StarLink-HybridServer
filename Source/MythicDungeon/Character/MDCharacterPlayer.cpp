// Copyright Epic Games, Inc. All Rights Reserved.

#include "MDCharacterPlayer.h"

#include "ASC/MDAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Physics/MDCollision.h"
#include "Player/MDPlayerState.h"

AMDCharacterPlayer::AMDCharacterPlayer()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(CPROFILE_MDPLAYER);

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// GAS
	AMDPlayerState* PS = GetPlayerState<AMDPlayerState>();
	if (PS)
	{
		ASC = Cast<UMDAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AttributeSet = PS->GetCharacterAttributeSet();
	}
}

void AMDCharacterPlayer::InitializeAbilitySystemComponent()
{
	Super::InitializeAbilitySystemComponent();

	if (!ASC || bAbilitySystemInitialized)
	{
		return;
	}
	
	// Initialize the ASC with owner and avatar
	ASC->InitAbilityActorInfo(GetPlayerState(), this);

	bAbilitySystemInitialized = true;
}

void AMDCharacterPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Server
	if (auto* PS = GetPlayerState<AMDPlayerState>())
	{
		ASC = Cast<UMDAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	}

	// Server: Initialize ASC, Abilities
	if (ASC && !bAbilitySystemInitialized)
	{
		InitializeAbilitySystemComponent();
		InitializeAbilities();
	}
}

void AMDCharacterPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (auto* PS = GetPlayerState<AMDPlayerState>())
	{
		ASC = Cast<UMDAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	}
	
	// Client: Initialize ASC
	if (ASC && !bAbilitySystemInitialized)
	{
		InitializeAbilitySystemComponent();
	}
}