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
	void OnHandbrakeInput(const FInputActionValue& InValue);
	void OnSteeringInput(const FInputActionValue& InValue);
	void OnJumpingInput(const FInputActionValue& InValue);
	void OnForwardFlipInput(const FInputActionValue& InValue);
	void OnLateralFlipInput(const FInputActionValue& InValue);

protected:
	//Input Config
	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	class UInputMappingContext* InputMapping;
	UPROPERTY(EditDefaultsOnly, Category = "Player Input")
	class UPlayerInputConfig* PlayerInputConfig;
	
};
