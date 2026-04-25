// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleMovementComponent.h"
#include "Components/PrimitiveComponent.h"

UE_DISABLE_OPTIMIZATION

// ─────────────────────────────────────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────────────────────────────────────
static constexpr float RAD_TO_RPM = 60.f / TWO_PI;
static constexpr float RPM_TO_RAD = TWO_PI / 60.f;
static constexpr float AIR_DENSITY = 1.225f;           // kg/m³
static constexpr float UU_TO_M = 0.01f;            // Unreal units → metres
static constexpr float M_TO_UU = 100.f;
static constexpr float FORCE_TO_UU = M_TO_UU;          // N → Unreal force units

UVehicleMovementComponent::UVehicleMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    //Initializing Editable Variables
    MaxSteeringAngle = 30.f;
    COMOffset = FVector(-5.f, 0.f, -10.f);
    DriveLayout = EDriveLayout::RearWheelDrive;
    MaxBrakingTorque = 2000.0f;
    ThrottleAcceleratingThreshold = 0.5;

    //Initializing State Variables
    ThrottleInput = 0.f;
    BrakeInput = 0.f;
    SteeringInput = 0.f;
    bHandbrake = false;

    Chassis = nullptr;
    CurrentGear = 0;
    VehicleSpeed = 0.0f;
    bIsGrounded = false;
    EffTorqueAtWheel = 0.0f;
    EffBrakingTorque = 0.0f;
}

void UVehicleMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache the chassis (must be a UPrimitiveComponent root)
    Chassis = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    ensureMsgf(Chassis, TEXT("VehicleSimComponent: Root must be a UPrimitiveComponent"));

    EngineRPM = EngineConfig.IdleRPM;
    CurrentGear = 0;

    // Physics setup — we handle all drag ourselves
    Chassis->SetLinearDamping(0.01f);
    Chassis->SetAngularDamping(0.01f);
    Chassis->SetCenterOfMass(COMOffset);
    Chassis->SetSimulatePhysics(true);

    EngineRPM = EngineConfig.IdleRPM;
}

void UVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Fn)
{
    Super::TickComponent(DeltaTime, TickType, Fn);

    if (!Chassis)
        return;

    // Convenience: speed in m/s from chassis linear velocity (Unreal → metres)
    VehicleSpeed = Chassis->GetPhysicsLinearVelocity().Size() * UU_TO_M;

    //Check grounding for Each Wheel and if the vehicle is grounded
    CheckGrounding();
    //Update Suspension Forces that lift the car above the ground
    UpdateSuspension();     
    //Update the Current Engine RPM
    UpdateEngineRPM();
    //Calculate the Torque transferred from Engine to Each Wheel
    CalculateTorqueAtEachDrivenWheel();
    //Apply Longitudinal and Lateral forces from Wheel friction with ground
    ApplyForceFromWheels();
    //Update Each Wheel Angular Speed based on all forces applied on it
    UpdateWheelsAngularSpeed(DeltaTime);
    //Perform Automatic Gear Shifting When needed
    ApplyAutomaticGearShifting();
}

//Check Grounding For each Wheel
void UVehicleMovementComponent::CheckGrounding()
{
    if (Chassis)
    {
        int32 NumOfWheelsGrounded = 0;

        //Ray Trace from each wheel origin, determine wheel state
        for (FVehicleWheelState& Wheel : Wheels)
        {
            FTransform CurrentWheelTransform = Wheel.OffsetTransform * Chassis->GetComponentTransform();
            FVector StartLocation = CurrentWheelTransform.GetLocation();
            FVector EndLocation = StartLocation - (Wheel.SpringRestLength + Wheel.GroundCheckTolerance) * Chassis->GetUpVector();
            FHitResult WheelTraceResult;
            FCollisionQueryParams WheelTraceParams("WheelTrace");
            WheelTraceParams.AddIgnoredActor(GetOwner());
            if (GetWorld()->LineTraceSingleByChannel(WheelTraceResult, StartLocation, EndLocation, ECC_Visibility, WheelTraceParams))
            {
                Wheel.bGrounded = true;
                Wheel.DistanceToGround = WheelTraceResult.Distance;
                Wheel.ContactPointNormal = WheelTraceResult.ImpactNormal;
                NumOfWheelsGrounded++;
            }
            else
            {
                Wheel.bGrounded = false;
                Wheel.DistanceToGround = FLT_MAX;
                Wheel.ContactPointNormal = FVector::ZeroVector;
            }
        }

        bIsGrounded = NumOfWheelsGrounded >= (Wheels.Num()) / 2; //Consider the vehicle grounded if more than half the wheels are grounded
    }
}

//Suspension trace and normal force to lift the car
void UVehicleMovementComponent::UpdateSuspension()
{
    //Apply Suspension at each grounded Wheel
    if (Chassis)
    {
        //Apply Suspension Spring Force at Each wheel base
        for (FVehicleWheelState& Wheel : Wheels)
        {
            if (Wheel.bGrounded)
            {
                FTransform CurrentWheelTransform = Wheel.OffsetTransform * Chassis->GetComponentTransform();

                float Compression = Wheel.SpringRestLength - Wheel.DistanceToGround;
                float SpringForce = Wheel.SpringStiffness * Compression;

                FVector WheelPhysVel = Chassis->GetPhysicsLinearVelocityAtPoint(CurrentWheelTransform.GetLocation());
                float WheelPhysVerticalVel = Chassis->GetUpVector().Dot(WheelPhysVel);
                float DampingForce = Wheel.SpringDamping * WheelPhysVerticalVel;

                FVector TotalAppliedSpringForce = Chassis->GetUpVector() * (SpringForce - DampingForce);

                Wheel.ContactUpForce = TotalAppliedSpringForce;

                Chassis->AddForceAtLocation(TotalAppliedSpringForce, CurrentWheelTransform.GetLocation());

                DrawDebugLine(GetWorld(), CurrentWheelTransform.GetLocation(), CurrentWheelTransform.GetLocation() + TotalAppliedSpringForce * 0.05f, FColor::Red, false);
            }
        }
    }
}

//Update the EngineRPM based on Wheels RPM
void UVehicleMovementComponent::UpdateEngineRPM()
{
    float AverageWheelsAngularSpeed = 0.0f;
    int32 NumOfDrivenWheels = 0;
    for (const FVehicleWheelState& Wheel : Wheels)
    {
        if (IsDrivenWheel(Wheel))
        {
            AverageWheelsAngularSpeed += Wheel.WheelAngularSpeed;
            NumOfDrivenWheels++;
        }
    }
    AverageWheelsAngularSpeed /= NumOfDrivenWheels;
    EngineRPM = FMath::Clamp((AverageWheelsAngularSpeed * EngineConfig.GearRatios[CurrentGear] * EngineConfig.FinalDrive) / (2 * UE_PI * 60.0f), EngineConfig.IdleRPM, EngineConfig.MaxRPM); //Convert Average Driven Wheels Angular Speed to EngineRPM
}

void UVehicleMovementComponent::CalculateTorqueAtEachDrivenWheel()
{
    //Get Torque Produced by Engine at current RPM
    float EngineTorque = ThrottleInput * EngineConfig.TorqueCurve.GetRichCurve()->Eval(EngineRPM) * (1 - ThrottleInput) * EngineConfig.EngineBrakeCurve.GetRichCurve()->Eval(EngineRPM);
    //Calculate Torque delivered to DriveShaft
    float DriveShaftTorque = EngineTorque * EngineConfig.GearRatios[CurrentGear] * EngineConfig.FinalDrive * EngineConfig.DrivetrainEfficiency;
    //Calculate the Torque delivered to each driven wheel
    int32 NumOfDrivenWheels = 0;
    for (const FVehicleWheelState& Wheel : Wheels)
    {
        if (IsDrivenWheel(Wheel))
        {
            NumOfDrivenWheels++;
        }
    }
    EffTorqueAtWheel = DriveShaftTorque / NumOfDrivenWheels;

    //Calculate The braking Torque by applying Brake Input to Max Braking Torque
    EffBrakingTorque = BrakeInput * MaxBrakingTorque;
}

void UVehicleMovementComponent::ApplyForceFromWheels()
{
    if (Chassis)
    {
        for (FVehicleWheelState& Wheel : Wheels)
        {
            //Get the Force Applied from Ground on contact point
            FVector UpForce = FMath::Max(Wheel.ContactUpForce, FVector::ZeroVector);
            //Get Wheel Speed
            float WheelSpeed = Wheel.WheelAngularSpeed * Wheel.WheelRadius;
            //Get Vehicle Velocity in the Forward Direction of the Wheel
        }
    }
}

void UVehicleMovementComponent::UpdateWheelsAngularSpeed(float DeltaSeconds)
{

}

void UVehicleMovementComponent::ApplyAutomaticGearShifting()
{
    float UpshiftRPM;
    float DownshiftRPM;
    if (ThrottleInput > ThrottleAcceleratingThreshold)
    {
        UpshiftRPM = FMath::Lerp(EngineConfig.CruisingUpshiftRPM, EngineConfig.AcceleratingUpshiftRPM, ThrottleInput);
        DownshiftRPM = FMath::Lerp(EngineConfig.CruisingDownshiftRPM, EngineConfig.AcceleratingDownshiftRPM, ThrottleInput);
    }
    else
    {
        UpshiftRPM = EngineConfig.CruisingUpshiftRPM;
        DownshiftRPM = EngineConfig.CruisingDownshiftRPM;
    }

    if (EngineRPM > UpshiftRPM && CurrentGear < (EngineConfig.GearRatios.Num() - 1))
        CurrentGear++;
    else if (EngineRPM < DownshiftRPM and CurrentGear > 0)
        CurrentGear--;
}


//Helper Functions
bool UVehicleMovementComponent::IsDrivenWheel(const FVehicleWheelState& Wheel)
{
    return DriveLayout == EDriveLayout::AllWheelDrive || (Wheel.bIsFrontWheel && DriveLayout == EDriveLayout::FrontWheelDrive) || (!Wheel.bIsFrontWheel && DriveLayout == EDriveLayout::RearWheelDrive);
}
UE_ENABLE_OPTIMIZATION
