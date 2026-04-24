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

    // ── Identity ──────────────────────────────
    UPROPERTY(EditAnywhere)
    bool bIsFrontWheel;
    UPROPERTY(EditAnywhere)
    bool bIsLeftWheel;

    // ── Suspension ────────────────────────────
    UPROPERTY(EditAnywhere)
    FVector SuspensionOffset;

    UPROPERTY(EditAnywhere)
    float SpringStiffness;  // N/m
    UPROPERTY(EditAnywhere)
    float DamperCoeff;   // N·s/m
    UPROPERTY(EditAnywhere)
    float RestLength;    // m
    UPROPERTY(EditAnywhere)
    float MinLength;    // m
    UPROPERTY(EditAnywhere)
    float MaxLength;    // m

    // ── Geometry ──────────────────────────────
    UPROPERTY(EditAnywhere)
    float WheelRadius;   // m
    UPROPERTY(EditAnywhere)
    float WheelInertia;    // kg·m²

    // ── Grip curve ────────────────────────────
    UPROPERTY(EditAnywhere)
    FRuntimeFloatCurve LateralGripCurve;

    // ── Frame state (written every tick) ──────
    bool bIsGrounded;
    FVector ContactPoint;
    FVector ContactNormal;
    float SuspensionCompression;     // current compression  (m)
    float PrevSuspensionComp;     // previous frame compression (for damper)
    float NormalForce;     // N  (spring + damper resultant)

    float WheelAngularVelocity;     // rad/s  (positive = forward)
    float WheelRPM;
    bool bIsLocked;

    FVector WheelForwardVector;
    FVector WheelRightVector;

    FVehicleWheelState()
    {
        //Initialize Editable Variables
        bIsFrontWheel = false;
        bIsLeftWheel = false;
        SuspensionOffset = FVector::ZeroVector;
        SpringStiffness = 35000.f;
        DamperCoeff = 4000.f; 
        RestLength = 0.45f;   
        MinLength = 0.15f;    
        MaxLength = 0.60f;    
        WheelRadius = 0.33f;  
        WheelInertia = 0.9f;  

        //Initializing state Variables
        bIsGrounded = false;
        ContactPoint = FVector::ZeroVector;
        ContactNormal = FVector::UpVector;
        SuspensionCompression = 0.0f;
        PrevSuspensionComp = 0.0f;
        NormalForce = 0.0f;
        WheelAngularVelocity = 0.0f;
        WheelRPM = 0.0f;
        bIsLocked = false;
        WheelForwardVector = FVector::ForwardVector;
        WheelRightVector = FVector::RightVector;
    }
};

USTRUCT(BlueprintType)
struct FVehicleEngineConfig
{
    GENERATED_BODY()

    /** Maps RPM → peak torque (Nm).  Throttle scales this linearly. */
    UPROPERTY(EditAnywhere)
    FRuntimeFloatCurve TorqueCurve;

    UPROPERTY(EditAnywhere)
    float IdleRPM;
    UPROPERTY(EditAnywhere)
    float MaxRPM;
    UPROPERTY(EditAnywhere)
    float EngineInertia;   // kg·m²
    UPROPERTY(EditAnywhere)
    float DrivetrainEfficiency;   // 0-1
    UPROPERTY(EditAnywhere)
    float DifferentialRatio;

    /** Gear ratios: index 0 = 1st gear */
    UPROPERTY(EditAnywhere)
    TArray<float> GearRatios;

    UPROPERTY(EditAnywhere)
    float MaxBrakeTorque;  // Nm total
    UPROPERTY(EditAnywhere)
    float FrontBrakeBias;   // fraction going to front
    UPROPERTY(EditAnywhere)
    float MuPeak;    // peak long friction coeff
    UPROPERTY(EditAnywhere)
    float MuKinetic;   // locked-wheel friction coeff
    UPROPERTY(EditAnywhere)
    float RollingResistanceCoeff;  // Crr
    UPROPERTY(EditAnywhere)
    float AeroDragCoeff;   // Cd
    UPROPERTY(EditAnywhere)
    float FrontalArea;    // m²

    UPROPERTY(EditAnywhere)
    EDriveLayout DriveLayout;

    FVehicleEngineConfig()
    {
        //Initializing Editable Variables
        IdleRPM = 800.0f;
        MaxRPM = 7000.0f;
        EngineInertia = 0.2f;
        DrivetrainEfficiency = 0.85f;
        DifferentialRatio = 3.7f;
        GearRatios = { 3.5f, 2.1f, 1.4f, 1.0f, 0.75 };
        MaxBrakeTorque = 4000.f;
        FrontBrakeBias = 0.70f;
        MuPeak = 1.2f;    
        MuKinetic = 0.75f;
        RollingResistanceCoeff = 0.015f;
        AeroDragCoeff = 0.35f;
        FrontalArea = 2.2f;
        DriveLayout = EDriveLayout::RearWheelDrive;
    }
};
