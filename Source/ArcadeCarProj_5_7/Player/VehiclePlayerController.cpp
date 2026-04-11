// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePlayerController.h"
#include "InputMappingContext.h"
#include "Input/PlayerInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

//Setup Input Component Using Enhanced Input Component
void AVehiclePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (EnhancedInputSubsystem && InputComponent)
	{
		EnhancedInputSubsystem->ClearAllMappings();
		EnhancedInputSubsystem->AddMappingContext(InputMapping, 0);
		UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
		if (EIC)
		{
			//Setup Of Various Input Configs from Player Input Configs to Call certain methods
			EIC->BindAction(PlayerInputConfig->Throttle, ETriggerEvent::Triggered, this, &AVehiclePlayerController::OnThrottleInput);
			EIC->BindAction(PlayerInputConfig->Steer, ETriggerEvent::Triggered, this, &AVehiclePlayerController::OnHandbrakeInput);
			EIC->BindAction(PlayerInputConfig->HandBreak, ETriggerEvent::Triggered, this, &AVehiclePlayerController::OnSteeringInput);
			EIC->BindAction(PlayerInputConfig->Jump, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnJumpingInput);
			EIC->BindAction(PlayerInputConfig->FlipForward, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnForwardFlipInput);
			EIC->BindAction(PlayerInputConfig->FlipLateral, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnLateralFlipInput);
		}
	}
}

//Input Handling Functions
void AVehiclePlayerController::OnThrottleInput(const FInputActionValue& InValue)
{
	FVector2D ThrottleValue = InValue.Get<FVector2D>();
}

void AVehiclePlayerController::OnHandbrakeInput(const FInputActionValue& InValue)
{
	bool bUsingHandbrake = InValue.Get<bool>();
}

void AVehiclePlayerController::OnSteeringInput(const FInputActionValue& InValue)
{
	FVector2D SteeringValue = InValue.Get<FVector2D>();
}

void AVehiclePlayerController::OnJumpingInput(const FInputActionValue& InValue)
{
	bool bStartedJump = InValue.Get<bool>();
}

void AVehiclePlayerController::OnForwardFlipInput(const FInputActionValue& InValue)
{
	FVector2D ForwardFlipDirection = InValue.Get<FVector2D>();
}

void AVehiclePlayerController::OnLateralFlipInput(const FInputActionValue& InValue)
{
	FVector2D LateralFlipDirection = InValue.Get<FVector2D>();
}