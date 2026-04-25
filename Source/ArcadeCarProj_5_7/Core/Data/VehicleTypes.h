// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "VehicleTypes.generated.h"

UENUM(BlueprintType)
enum class EDriveLayout : uint8
{
    FrontWheelDrive,
    RearWheelDrive,
    AllWheelDrive
};

enum ETransmissionMode
{
    Neutral,
    Drive,
    Reverse,
    Park,
};

USTRUCT(BlueprintType)
struct FVehicleWheelState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Wheel Config")
    FTransform OffsetTransform;
    UPROPERTY(EditAnywhere, Category = "Wheel Config")
    bool bIsFrontWheel;
    UPROPERTY(EditAnywhere, Category = "Wheel Config")
    float WheelRadius;
    UPROPERTY(EditAnywhere, Category = "Wheel Config")
    float WheelInertia;

    UPROPERTY(EditAnywhere, Category = "Suspension")
    float GroundCheckTolerance;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringRestLength;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringStiffness;
    UPROPERTY(EditAnywhere, Category = "Suspension")
    float SpringDamping;

    UPROPERTY(EditAnywhere, Category = "Longitudinal Force")
    float miuLongPeak;   //Peak Longitudinal Friction
    UPROPERTY(EditAnywhere, Category = "Longitudinal Force")
    float miuLongSlide;   //Sliding Longitudinal Friction
    UPROPERTY(EditAnywhere, Category = "Longitudinal Force")
    float SlipRatioPeak;   //Slip Ratio At Peak Force
    UPROPERTY(EditAnywhere, Category = "Longitudinal Force")
    float SlipRatioLongRange;    //Slip range over which force drops to sliding; used to compute FalloffRate_x

    UPROPERTY(EditAnywhere, Category = "Lateral Force")
    float miuLatPeak;   //Peak Lateral Friction
    UPROPERTY(EditAnywhere, Category = "Lateral Force")
    float miuLatSlide;   //Sliding Lateral Friction
    UPROPERTY(EditAnywhere, Category = "Lateral Force")
    float SlipAnglePeak;   //Slip Angle At Peak Force
    UPROPERTY(EditAnywhere, Category = "Lateral Force")
    float SlipAngleLatRange;    //Angle range over which force drops to sliding; derives FalloffRate_y

    // State Variables
    bool bGrounded;
    float DistanceToGround;
    FVector ContactPointNormal;
    FVector ContactUpForce;
    float SteeringAngle;

    FVehicleWheelState()
    {
        //Initialize Editable Variables
        bIsFrontWheel = false;
        WheelRadius = 0.35f;
        WheelInertia = 1.5f;
        GroundCheckTolerance = 5.0f;
        SpringRestLength = 30.0f;
        SpringStiffness = 50000.0f;
        SpringDamping = 7000.0f;
        miuLongPeak = 0.9f;  
        miuLongSlide = 0.75f; 
        SlipRatioPeak = 0.1f;
        SlipRatioLongRange = 0.2f;
        miuLatPeak = 0.95f;  
        miuLatSlide = 0.8f;  
        SlipAnglePeak = 0.14f;
        SlipAngleLatRange = 0.1f;  

        //Initializing state Variables
        OffsetTransform = FTransform();
        bGrounded = false;
        DistanceToGround = false;
        ContactPointNormal = FVector::UpVector;
        ContactUpForce = FVector::UpVector;
        SteeringAngle = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FVehicleEngineConfig
{
    GENERATED_BODY()

    //Tuning Variables
    UPROPERTY(EditAnywhere, Category = "Engine Block")
    FRuntimeFloatCurve TorqueCurve;
    UPROPERTY(EditAnywhere, Category = "Engine Block")
    FRuntimeFloatCurve EngineBrakeCurve;
    UPROPERTY(EditAnywhere, Category = "Engine Block")
    float IdleRPM;
    UPROPERTY(EditAnywhere, Category = "Engine Block")
    float MaxRPM;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    TArray<float> GearRatios;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    float FinalDrive;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    float DrivetrainEfficiency;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    float CruisingUpshiftRPM;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    float AcceleratingUpshiftRPM;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    float CruisingDownshiftRPM;
    UPROPERTY(EditAnywhere, Category = "Gearbox")
    float AcceleratingDownshiftRPM;


    FVehicleEngineConfig()
    {
        IdleRPM = 900.0f;
        MaxRPM = 8000.0f;
        GearRatios = { 3.5f, 2.4f, 1.8f, 1.3f, 1.0f, 0.8f };
        FinalDrive = 3.7f;
        DrivetrainEfficiency = 0.85f;
        CruisingUpshiftRPM = 3000.0f;
        AcceleratingUpshiftRPM = 6500.f;
        CruisingDownshiftRPM = 1500.0f;
        AcceleratingDownshiftRPM = 3000.0f;
    }
};
