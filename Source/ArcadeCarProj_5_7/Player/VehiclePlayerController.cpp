// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePlayerController.h"
#include "InputMappingContext.h"
#include "Input/PlayerInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "ArcadeCarProj_5_7/Vehicle/VehiclePawn.h"

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
			EIC->BindAction(PlayerInputConfig->Throttle, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnThrottleInputReleased);
			EIC->BindAction(PlayerInputConfig->HandBrake, ETriggerEvent::Triggered, this, &AVehiclePlayerController::OnHandbrakeInput);
			EIC->BindAction(PlayerInputConfig->Steer, ETriggerEvent::Triggered, this, &AVehiclePlayerController::OnSteeringInput);
			EIC->BindAction(PlayerInputConfig->Steer, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnSteeringInputReleased);
			EIC->BindAction(PlayerInputConfig->Jump, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnJumpingInput);
			EIC->BindAction(PlayerInputConfig->FlipForward, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnForwardFlipInput);
			EIC->BindAction(PlayerInputConfig->FlipLateral, ETriggerEvent::Completed, this, &AVehiclePlayerController::OnLateralFlipInput);
			EIC->BindAction(PlayerInputConfig->SetNeutralTransmission, ETriggerEvent::Started, this, &AVehiclePlayerController::OnSetNeutralTransmission);
			EIC->BindAction(PlayerInputConfig->SetDriveTransmission, ETriggerEvent::Started, this, &AVehiclePlayerController::OnSetDriveTransmission);
			EIC->BindAction(PlayerInputConfig->SetReverseTransmission, ETriggerEvent::Started, this, &AVehiclePlayerController::OnSetReverseTransmission);
			EIC->BindAction(PlayerInputConfig->SetParkTransmission, ETriggerEvent::Started, this, &AVehiclePlayerController::OnSetParkTransmission);
			EIC->BindAction(PlayerInputConfig->GearUpShift, ETriggerEvent::Started, this, &AVehiclePlayerController::OnGearUpShift);
			EIC->BindAction(PlayerInputConfig->GearDownShift, ETriggerEvent::Started, this, &AVehiclePlayerController::OnGearDownShift);
		}
	}
}

//Input Handling Functions
void AVehiclePlayerController::OnThrottleInput(const FInputActionValue& InValue)
{
	float ThrottleValue = InValue.Get<float>();
	//Get the Vehicle Pawn and Set Target throttle Force
	if (GetVehiclePawn())
	{
		GetVehiclePawn()->SetTargetThrottleInput(ThrottleValue);
	}
}

void AVehiclePlayerController::OnThrottleInputReleased(const FInputActionValue& InValue)
{
	//Set Target Throttle value to neutral in Vehicle Pawn
	if (GetVehiclePawn())
	{
		GetVehiclePawn()->SetTargetThrottleInput(0.0f);
	}
}

void AVehiclePlayerController::OnHandbrakeInput(const FInputActionValue& InValue)
{
	bool bUsingHandbrake = InValue.Get<bool>();
}

void AVehiclePlayerController::OnSteeringInput(const FInputActionValue& InValue)
{
	float SteeringValue = InValue.Get<float>();
	//Set Target Steering value in Vehicle Pawn
	if (GetVehiclePawn())
	{
		GetVehiclePawn()->SetTargetSteeringValue(SteeringValue);
	}
}

void AVehiclePlayerController::OnSteeringInputReleased(const FInputActionValue& InValue)
{
	//Set Target Steering value to neutral in Vehicle Pawn
	if (GetVehiclePawn())
	{
		GetVehiclePawn()->SetTargetSteeringValue(0.0f);
	}
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

void AVehiclePlayerController::OnSetNeutralTransmission(const FInputActionValue& InValue)
{
	bool bSetPressed = InValue.Get<bool>();
	if (bSetPressed && GetVehiclePawn())
	{
		GetVehiclePawn()->SetCurrentTransmissionMode(ETransmissionMode::Neutral);
	}
}

void AVehiclePlayerController::OnSetDriveTransmission(const FInputActionValue& InValue)
{
	bool bSetPressed = InValue.Get<bool>();
	if (bSetPressed && GetVehiclePawn())
	{
		GetVehiclePawn()->SetCurrentTransmissionMode(ETransmissionMode::Drive);
	}
}

void AVehiclePlayerController::OnSetReverseTransmission(const FInputActionValue& InValue)
{
	bool bSetPressed = InValue.Get<bool>();
	if (bSetPressed && GetVehiclePawn())
	{
		GetVehiclePawn()->SetCurrentTransmissionMode(ETransmissionMode::Reverse);
	}
}

void AVehiclePlayerController::OnSetParkTransmission(const FInputActionValue& InValue)
{
	bool bSetPressed = InValue.Get<bool>();
	if (bSetPressed && GetVehiclePawn())
	{
		GetVehiclePawn()->SetCurrentTransmissionMode(ETransmissionMode::Park);
	}
}

void AVehiclePlayerController::OnGearUpShift(const FInputActionValue& InValue)
{
	bool bShfitPressed = InValue.Get<bool>();
	if (bShfitPressed && GetVehiclePawn())
	{
		GetVehiclePawn()->DoGearUpShift();
	}
}

void AVehiclePlayerController::OnGearDownShift(const FInputActionValue& InValue)
{
	bool bShfitPressed = InValue.Get<bool>();
	if (bShfitPressed && GetVehiclePawn())
	{
		GetVehiclePawn()->DoGearDownShift();
	}
}

AVehiclePawn* AVehiclePlayerController::GetVehiclePawn()
{
	if (!VehiclePawn)	//If reference to vehicle pawn isn't valid, cast from current pawn to vehicle pawn
	{
		VehiclePawn = Cast<AVehiclePawn>(GetPawn());
	}
	return VehiclePawn;
}