// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehicleInterface.h"
#include "VehiclePawn.generated.h"

UENUM(BlueprintType)
enum EDriveTrainType : uint8
{
	FWD, 
	RWD,
	AWD,
};

enum EWheelType : uint8
{
	None,
	FrontWheel,
	RearWheel,
};

struct FWheelState
{
	FString WheelName;
	FTransform OffsetTransform;
	bool bGrounded;
	float DistanceToGround;
	FVector ContactPointNormal;
	float SteeringAngle;
	EWheelType WheelType;
	bool bDrivingWheel;

	FWheelState()
	{
		WheelName = "";
		OffsetTransform = FTransform();
		bGrounded = false;
		DistanceToGround = FLT_MAX;
		ContactPointNormal = FVector::ZeroVector;
		SteeringAngle = 0.0f;
		WheelType = EWheelType::None;
		bDrivingWheel = false;
	}
};

UCLASS()
class ARCADECARPROJ_5_7_API AVehiclePawn : public APawn, public IVehicleInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehiclePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//Async Physics Tick, used to update physics calculations at fixed intervals(Determined from project settings)
	virtual void AsyncPhysicsTickActor(float DeltaTime, float SimTime) override;

private:
	//Wheel Functions
	void InitializeAttachedWheels();
	void CheckGrounding();
	void ApplySuspensionForces();
	void UpdateThrottle(float deltaSeconds);
	void UpdateEngineRPM(float deltaSeconds);
	void ApplyEngineTorque();
	void DistributeForceToDrivingWheels(float ThrottleForce);
	void UpdateAndApplySteering(float deltaSeconds);
	bool IsTransmissionEngaged();

public:
	//Input Related Functions
	virtual void SetTargetThrottleInput(float ThrottleValue) override;
	virtual void SetTargetSteeringValue(float InSteeringValue) override;
	virtual void SetUsingHandbrake(bool InUseHandbrake) override { bApplyHandbrake = InUseHandbrake; }
	virtual void SetCurrentTransmissionMode(ETransmissionMode InTransmissionMode) override { CurrentTransmissionMode = InTransmissionMode; }
	virtual void DoGearUpShift() override
	{
		if (CurrentTransmissionMode == ETransmissionMode::Drive && CurrentGearIndex < (GearRatios.Num() - 1))
			CurrentGearIndex++;
	}
	virtual void DoGearDownShift() override
	{
		if (CurrentTransmissionMode == ETransmissionMode::Drive && CurrentGearIndex > 0)
			CurrentGearIndex--;
	}

	//Vehicle Movement Information Functions
	float GetCurrentForwardSpeedKMH();
	float GetCurrentRPM();

protected:
	//Vehicle Pawn Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyComponents")
	UStaticMeshComponent* VehicleBody;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BodyComponents")
	TArray<UStaticMeshComponent*> VehicleWheelComponents;

	//Vehicle Movement Tunning
	UPROPERTY(EditAnywhere, Category = "Grounding")
	float GroundCheckTolerance;

	UPROPERTY(EditAnywhere, Category = "Suspension")
	float SpringRestLength;
	UPROPERTY(EditAnywhere, Category = "Suspension")
	float SpringStiffness;
	UPROPERTY(EditAnywhere, Category = "Suspension")
	float SpringDamping;

	UPROPERTY(EditAnywhere, Category = "Throttle")
	TEnumAsByte<EDriveTrainType> DriveTrainType;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float ThrottleValueChangeSpeed;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float MaxEngineTorque;
	UPROPERTY(EditAnywhere, Category = "Throttle")	//X: RPM , Y: Torque 
	FRuntimeFloatCurve RPMTorqueCurve;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float RevRate;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float EngineFrictionRate;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float IdleRPM;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float RedlineRPM;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	TArray<float> GearRatios;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float ReverseGearRatio;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float FinalDriveTrainRatio;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float DriveTrainEfficiency;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float StandardWheelRadius;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float MaxForwardSpeed;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float MaxReverseSpeed;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float MaxSteeringAngleDegrees;
	UPROPERTY(EditAnywhere, Category = "Steering")
	float SteeringAngularSpeedFrac;
	UPROPERTY(EditAnywhere, Category = "Steering")
	FRuntimeFloatCurve FrontSpeedGripCurve;
	UPROPERTY(EditAnywhere, Category = "Steering")
	FRuntimeFloatCurve RearSpeedGripCurve;

private:
	//Vehicle Movement Variables
	TArray<FWheelState> WheelStates;
	ETransmissionMode CurrentTransmissionMode;
	bool bIsGrounded;
	float TargetThrottleValue;
	float CurrentThrottleValue;
	float TargetSteeringValue;
	float CurrentSteeringValue;
	bool bApplyHandbrake;
	float CurrentRPM;
	int32 CurrentGearIndex;
};
