// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArcadeCarProj_5_7/Core/Data/VehicleTypes.h"
#include "VehicleMovementComponent.generated.h"


UCLASS( ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent) )
class ARCADECARPROJ_5_7_API UVehicleMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UVehicleMovementComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    //Input Related Functions
    void SetThrottleInput(float InValue) { ThrottleInput = InValue; }
    void SetBrakeInput(float InValue) { BrakeInput = InValue; }
    void SetSteeringInput(float InValue) { SteeringInput = InValue; }
    void SetHandbrakeInput(bool bValue) { bHandbrake = bValue; }

    //Internal Component Getters and Setters
    FVehicleEngineConfig& GetEngineConfig() { return EngineConfig; }
    int32 GetCurrentGear() const { return CurrentGear; }
    void SetCurrentGear(int32 InValue) { CurrentGear = InValue; }

private:
    // ── Internal sub-steps ────────────────────────────────────────────────
    void  UpdateSuspension(float DeltaTime);
    void  UpdateSteeringVectors();
    void  UpdateWheelAngularVel(FVehicleWheelState& W, float DeltaTime);
    void  UpdateEngineRPM(float DeltaTime);
    void  ReconnectDrivenWheels();
    void  ApplyTireForces();
    void  ApplyLowSpeedStiction();

    // ── Helpers ───────────────────────────────────────────────────────────
    FVector GetWheelVelocity(const FVector& WheelWorldPos) const;
    float   ComputeSlipAngle(const FVehicleWheelState& W, const FVector& WheelVelocity) const;
    float   ComputeLongSlipRatio(const FVehicleWheelState& W, const FVector& WheelVelocity) const;
    float   LongFrictionFromSlip(float SlipRatio) const;
    float   LatFrictionFromAngle(const FVehicleWheelState& W, float SlipAngleDeg) const;
    bool    IsWheelDriven(const FVehicleWheelState& W) const;
    float   TotalDriveRatio() const;   // GearRatio * DiffRatio
    float   ComputeWheelResistanceTorque(const FVehicleWheelState& W) const;
    float   ComputeEngineBrakeTorque() const;

protected:
    //Vehicle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle")
    FVehicleEngineConfig EngineConfig;
    UPROPERTY(EditAnywhere, Category = "Vehicle")
    TArray<FVehicleWheelState> Wheels;
    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float MaxSteeringAngle;
    UPROPERTY(EditAnywhere, Category = "Vehicle")
    FVector COMOffset;


private:
    //Input Related variables
    float ThrottleInput = 0.f; // 0-1
    float BrakeInput = 0.f; // 0-1
    float SteeringInput = 0.f; // -1 left … +1 right
    bool  bHandbrake = false;

    //Movement Related Variables
    TObjectPtr<UPrimitiveComponent> Chassis;
    float EngineRPM = 800.f;
    int32 CurrentGear = 0;     // 0-based index
    float VehicleSpeed = 0.f;   // m/s	
};
