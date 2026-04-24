// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehicleMovementComponent.h"
#include "VehicleInterface.h"
#include "RealisticVehiclePawn.generated.h"

UCLASS()
class ARCADECARPROJ_5_7_API ARealisticVehiclePawn : public APawn, public IVehicleInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARealisticVehiclePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	//Input Related Variables
	virtual void SetTargetThrottleInput(float ThrottleValue) override { TargetThrottleValue = ThrottleValue; }
	virtual void SetTargetSteeringValue(float InSteeringValue) override { TargetSteeringValue = InSteeringValue; }
	virtual void SetUsingHandbrake(bool bInUseHandbrake) override
	{
		if (VehicleMovementComp)
		{
			VehicleMovementComp->SetHandbrakeInput(bInUseHandbrake);
		}
	}
	virtual void DoGearUpShift() override;
	virtual void DoGearDownShift() override;

private:
	//Update Inputs values with Speed and Set in VehicleMovementComp
	void UpdateThrottleInput(float deltaSeconds);
	void UpdateBrakingInput(float deltaSeconds);
	void UpdateSteeringInput(float deltaSeconds);

protected:
	//Pawn Components
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ChassisComp;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UVehicleMovementComponent* VehicleMovementComp;
	//Vehicle Input Config
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float ThrottleValueChangeSpeed;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float BrakeValueChangeSpeed;
	UPROPERTY(EditAnywhere, Category = "Steering")
	float SteeringAngularSpeedFrac;

private:
	//Input Related Variables
	float TargetThrottleValue;
	float CurrentThrottleValue;
	float TargetBrakeValue;
	float CurrentBrakeValue;
	float TargetSteeringValue;
	float CurrentSteeringValue;
};
