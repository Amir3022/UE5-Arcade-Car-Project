// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "VehiclePlayerController.generated.h"

UCLASS()
class ARCADECARPROJ_5_7_API AVehiclePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

private:
	//Input Handling Functions
	void OnThrottleInput(const FInputActionValue& InValue);
	void OnThrottleInputReleased(const FInputActionValue& InValue);
	void OnHandbrakeInput(const FInputActionValue& InValue);
	void OnSteeringInput(const FInputActionValue& InValue);
	void OnSteeringInputReleased(const FInputActionValue& InValue);
	void OnJumpingInput(const FInputActionValue& InValue);
	void OnForwardFlipInput(const FInputActionValue& InValue);
	void OnLateralFlipInput(const FInputActionValue& InValue);
	void OnSetNeutralTransmission(const FInputActionValue& InValue);
	void OnSetDriveTransmission(const FInputActionValue& InValue);
	void OnSetReverseTransmission(const FInputActionValue& InValue);
	void OnSetParkTransmission(const FInputActionValue& InValue);
	void OnGearUpShift(const FInputActionValue& InValue);
	void OnGearDownShift(const FInputActionValue& InValue);

	//Vehicle Pawn Functions
	class IVehicleInterface* GetVehiclePawn();

protected:
	//Input Config
	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	class UInputMappingContext* InputMapping;
	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	class UPlayerInputConfig* PlayerInputConfig;

private:
	//Vehicle Pawn Variables
	class IVehicleInterface* VehiclePawn;
};
