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
    void CheckGrounding();
    void UpdateSuspension();
    void UpdateEngineRPM();
    void CalculateTorqueAtEachDrivenWheel();
    void ApplyForceFromWheels();
    void UpdateWheelsAngularSpeed(float DeltaSeconds);
    void ApplyAutomaticGearShifting();

    // ── Helper Functions ──────────────────────────────────────────────────
    bool IsDrivenWheel(const FVehicleWheelState& Wheel);

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
    UPROPERTY(EditAnywhere, Category = "Vehicle")
    EDriveLayout DriveLayout;
    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float MaxBrakingTorque;
    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float ThrottleAcceleratingThreshold;


private:
    //Input Related variables
    float ThrottleInput; // 0-1
    float BrakeInput; // 0-1
    float SteeringInput; // -1 left … +1 right
    bool  bHandbrake;

    //State Variables
    TObjectPtr<UPrimitiveComponent> Chassis;
    float EngineRPM;
    int32 CurrentGear;     // 0-based index
    float VehicleSpeed;
    bool bIsGrounded;
    float EffTorqueAtWheel;
    float EffBrakingTorque;
};
