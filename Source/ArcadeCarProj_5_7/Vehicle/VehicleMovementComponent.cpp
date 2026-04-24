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

    //Initializing State Variables
    ThrottleInput = 0.f;
    BrakeInput = 0.f;
    SteeringInput = 0.f;
    bHandbrake = false;


    Chassis = nullptr;
    EngineRPM = 800.f;
    CurrentGear = 0;
    VehicleSpeed = 0.f;
}

void UVehicleMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache the chassis (must be a UPrimitiveComponent root)
    Chassis = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    ensureMsgf(Chassis, TEXT("VehicleSimComponent: Root must be a UPrimitiveComponent"));

    // Physics setup — we handle all drag ourselves
    Chassis->SetLinearDamping(0.01f);
    Chassis->SetAngularDamping(0.01f);
    Chassis->SetCenterOfMass(COMOffset);

    EngineRPM = EngineConfig.IdleRPM;
}

void UVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Fn)
{
    Super::TickComponent(DeltaTime, TickType, Fn);
    if (!Chassis)
        return;

    // Convenience: speed in m/s from chassis linear velocity (Unreal → metres)
    VehicleSpeed = Chassis->GetPhysicsLinearVelocity().Size() * UU_TO_M;

    UpdateSuspension(DeltaTime);       // 1 + 2
    UpdateSteeringVectors();           // 3

    for (FVehicleWheelState& W : Wheels)
        UpdateWheelAngularVel(W, DeltaTime); // 4 — brake / handbrake path

    UpdateEngineRPM(DeltaTime);        // 5
    ReconnectDrivenWheels();           // 6
    ApplyTireForces();                 // 7
    ApplyLowSpeedStiction();           // 8
}

//Suspension trace and normal force to lift the car
void UVehicleMovementComponent::UpdateSuspension(float DeltaTime)
{
    UWorld* World = GetWorld();

    for (FVehicleWheelState& W : Wheels)
    {
        // World-space suspension top
        FVector SuspTop = GetOwner()->GetActorTransform()
            .TransformPosition(W.SuspensionOffset * M_TO_UU);
        FVector TraceEnd = SuspTop - GetOwner()->GetActorUpVector()
            * (W.MaxLength + W.WheelRadius) * M_TO_UU;

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(GetOwner());

        W.bIsGrounded = World->LineTraceSingleByChannel(
            Hit, SuspTop, TraceEnd, ECC_Visibility, Params);

        W.PrevSuspensionComp = W.SuspensionCompression;

        if (W.bIsGrounded)
        {
            W.ContactPoint = Hit.ImpactPoint;
            W.ContactNormal = Hit.ImpactNormal;

            // How far the spring has compressed past rest length (metres)
            float TraceLen = (SuspTop - Hit.ImpactPoint).Size() * UU_TO_M - W.WheelRadius;
            W.SuspensionCompression = FMath::Clamp(
                W.MaxLength - TraceLen,
                0.f,
                W.MaxLength - W.MinLength);

            // Spring force
            float SpringForce = W.SpringStiffness * W.SuspensionCompression;

            // Damper force (velocity of compression)
            float CompVelocity = (W.SuspensionCompression - W.PrevSuspensionComp) / DeltaTime;
            float DamperForce = W.DamperCoeff * CompVelocity;

            W.NormalForce = FMath::Max(SpringForce + DamperForce, 0.f);

            // Apply upward force along contact normal
            FVector SuspForce = W.ContactNormal * W.NormalForce * FORCE_TO_UU;
            Chassis->AddForceAtLocation(SuspForce, W.ContactPoint);
        }
        else
        {
            W.SuspensionCompression = 0.f;
            W.NormalForce = 0.f;
            W.ContactPoint = FVector::ZeroVector;
            W.ContactNormal = FVector::UpVector;
        }
    }
}

//Get Current Steering vectors based on steering input
void UVehicleMovementComponent::UpdateSteeringVectors()
{
    const float SteerDeg = SteeringInput * MaxSteeringAngle;
    const FVector ChassisFwd = GetOwner()->GetActorForwardVector();
    const FVector ChassisUp = GetOwner()->GetActorUpVector();

    for (FVehicleWheelState& W : Wheels)
    {
        float Angle = W.bIsFrontWheel ? SteerDeg : 0.f;
        FQuat  SteerRot(ChassisUp, FMath::DegreesToRadians(Angle));
        W.WheelForwardVector = SteerRot.RotateVector(ChassisFwd).GetSafeNormal();
        W.WheelRightVector = FVector::CrossProduct(ChassisUp, W.WheelForwardVector)
            .GetSafeNormal() * -1.f;
    }
}

//Update Wheel angular velocity based on current throttle and brake inputs
void UVehicleMovementComponent::UpdateWheelAngularVel(FVehicleWheelState& W, float DeltaTime)
{
    // Determine brake torque for this wheel
    float AppliedBrakeTorque = 0.f;

    if (bHandbrake && !W.bIsFrontWheel)
    {
        AppliedBrakeTorque = 1e6f;   // near-infinite — clamp below
    }
    else if (BrakeInput > 0.01f)
    {
        float Bias = W.bIsFrontWheel
            ? EngineConfig.FrontBrakeBias
            : (1.f - EngineConfig.FrontBrakeBias);
        AppliedBrakeTorque = BrakeInput * EngineConfig.MaxBrakeTorque * Bias;
    }

    if (AppliedBrakeTorque < 0.01f) return; // driven path handled in ReconnectDrivenWheels

    // ── Road-reaction torque from current slip ────────────────────────────
    float RoadReactionTorque = 0.f;
    if (W.bIsGrounded)
    {
        FVector WheelVel = GetWheelVelocity(W.ContactPoint);
        float   LongSpeed = FVector::DotProduct(WheelVel, W.WheelForwardVector) * UU_TO_M;
        float   WheelSurf = W.WheelAngularVelocity * W.WheelRadius;
        float   Denom = FMath::Max(FMath::Abs(LongSpeed), 0.1f);
        float   SlipRatio = FMath::Clamp((WheelSurf - LongSpeed) / Denom, -1.f, 1.f);
        float   MuLong = LongFrictionFromSlip(SlipRatio);
        RoadReactionTorque = MuLong * W.NormalForce * W.WheelRadius;
    }

    // ── Net torque on wheel ───────────────────────────────────────────────
    float BrakeSign = (W.WheelAngularVelocity > 0.f) ? -1.f : 1.f;
    float NetTorque = (RoadReactionTorque * FMath::Sign(W.WheelAngularVelocity))
        + (AppliedBrakeTorque * BrakeSign);

    W.WheelAngularVelocity += (NetTorque / W.WheelInertia) * DeltaTime;
    W.WheelAngularVelocity = FMath::Max(W.WheelAngularVelocity, 0.f);

    // Lockup: brake can overpower road reaction
    W.bIsLocked = (AppliedBrakeTorque > FMath::Abs(RoadReactionTorque) * 1.1f);
    if (W.bIsLocked) W.WheelAngularVelocity = 0.f;

    W.WheelRPM = W.WheelAngularVelocity * RAD_TO_RPM;
}

void UVehicleMovementComponent::UpdateEngineRPM(float DeltaTime)
{
    if (!EngineConfig.TorqueCurve.GetRichCurve())
        return;

    // Engine torque from throttle + torque curve
    float PeakTorque = EngineConfig.TorqueCurve.GetRichCurve()->Eval(EngineRPM);
    float EngineTorque = PeakTorque * ThrottleInput;

    if(ThrottleInput > 0.99f)
        UE_LOG(LogTemp, Warning, TEXT("Amir, MoveComp Throttle: %f"), ThrottleInput);

    // Sum resistance from all driven wheels mapped back through drivetrain
    float TotalWheelResistance = 0.f;
    for (const FVehicleWheelState& W : Wheels)
    {
        if (IsWheelDriven(W))
            TotalWheelResistance += ComputeWheelResistanceTorque(W);
    }

    float Ratio = TotalDriveRatio();
    float LoadAtEngine = (TotalWheelResistance * Ratio) / EngineConfig.DrivetrainEfficiency;
    float InternalFriction = 15.f + EngineRPM * 0.005f;
    float EngineBraking = ComputeEngineBrakeTorque();

    float NetTorque = EngineTorque - LoadAtEngine - InternalFriction - EngineBraking;

    // Integrate: torque → angular acceleration → RPM
    float EngineAngVel = EngineRPM * RPM_TO_RAD;
    EngineAngVel += (NetTorque / EngineConfig.EngineInertia) * DeltaTime;
    EngineRPM = EngineAngVel * RAD_TO_RPM;
    EngineRPM = FMath::Clamp(EngineRPM, EngineConfig.IdleRPM, EngineConfig.MaxRPM);

    UE_LOG(LogTemp, Warning, TEXT("Amir, Current Engine RPM: %f"), EngineRPM);
}

//Connect between Wheels RPM and Current Engine RPM
void UVehicleMovementComponent::ReconnectDrivenWheels()
{
    for (FVehicleWheelState& W : Wheels)
    {
        if (!IsWheelDriven(W))
            continue;

        // Decouple when braking or handbrake active on rear
        bool bBraking = BrakeInput > 0.05f;
        bool bHandbrakeR = bHandbrake && !W.bIsFrontWheel;
        if (bBraking || bHandbrakeR)
            continue;

        // Lock wheel to engine through drivetrain
        float Ratio = TotalDriveRatio();
        W.WheelAngularVelocity = (EngineRPM * RPM_TO_RAD) / Ratio;
        W.WheelRPM = EngineRPM / Ratio;
        W.bIsLocked = false;
    }
}

//Apply Tire forward force to move the car forward or backwards based on current rotation direction
void UVehicleMovementComponent::ApplyTireForces()
{
    for (FVehicleWheelState& W : Wheels)
    {
        if (!W.bIsGrounded) continue;

        FVector WheelVel = GetWheelVelocity(W.ContactPoint);  // UU/s

        // ── Locked wheel — pure kinetic friction opposing velocity ────────
        if (W.bIsLocked)
        {
            FVector SlideDir = -(WheelVel.GetSafeNormal());
            FVector KineticF = SlideDir * EngineConfig.MuKinetic * W.NormalForce * FORCE_TO_UU;
            Chassis->AddForceAtLocation(KineticF, W.ContactPoint);
            continue;
        }

        // ── Longitudinal slip ratio ───────────────────────────────────────
        float LongSpeedMS = FVector::DotProduct(WheelVel, W.WheelForwardVector) * UU_TO_M;
        float WheelSurfMS = W.WheelAngularVelocity * W.WheelRadius;
        float Denom = FMath::Max(FMath::Abs(LongSpeedMS), 0.1f);
        float SlipRatio = FMath::Clamp((WheelSurfMS - LongSpeedMS) / Denom, -1.f, 1.f);
        float MuLong = LongFrictionFromSlip(SlipRatio);

        // ── Lateral slip angle ────────────────────────────────────────────
        float SlipAngle = ComputeSlipAngle(W, WheelVel);         // degrees
        float MuLat = LatFrictionFromAngle(W, SlipAngle);

        // ── Friction circle — combined grip budget ────────────────────────
        float Combined = FMath::Sqrt(MuLong * MuLong + MuLat * MuLat);
        if (Combined > EngineConfig.MuPeak)
        {
            float Scale = EngineConfig.MuPeak / Combined;
            MuLong *= Scale;
            MuLat *= Scale;
        }

        // ── Build force vectors ───────────────────────────────────────────
        FVector LongForce = W.WheelForwardVector * (MuLong * W.NormalForce);
        FVector LatForce = W.WheelRightVector * (MuLat * W.NormalForce) * -1.f;

        FVector TotalForce = (LongForce + LatForce) * FORCE_TO_UU;
        Chassis->AddForceAtLocation(TotalForce, W.ContactPoint);
    }
}

//Simulate drag at low speeds to get the car to a complete stop when applying brakes or leaving throttle
void UVehicleMovementComponent::ApplyLowSpeedStiction()
{
    float Speed = Chassis->GetPhysicsLinearVelocity().Size() * UU_TO_M;
    float AngularSpeed = Chassis->GetPhysicsAngularVelocityInRadians().Size();

    if (Speed < 0.5f && ThrottleInput < 0.01f && BrakeInput < 0.01f)
    {
        FVector DampedLin = Chassis->GetPhysicsLinearVelocity() * 0.85f;
        Chassis->SetPhysicsLinearVelocity(DampedLin);
    }

    if (AngularSpeed < 0.05f)
    {
        FVector DampedAng = Chassis->GetPhysicsAngularVelocityInRadians() * 0.80f;
        Chassis->SetPhysicsAngularVelocityInRadians(DampedAng);
    }
}

//Helper Functions
FVector UVehicleMovementComponent::GetWheelVelocity(const FVector& WheelWorldPos) const
{
    // v_point = v_COM + ω × r
    FVector AngVel = Chassis->GetPhysicsAngularVelocityInRadians();
    FVector Offset = WheelWorldPos - Chassis->GetCenterOfMass();
    return Chassis->GetPhysicsLinearVelocity() + FVector::CrossProduct(AngVel, Offset);
}

float UVehicleMovementComponent::ComputeSlipAngle(const FVehicleWheelState& W, const FVector& WheelVelocity) const
{
    float Vx = FVector::DotProduct(WheelVelocity, W.WheelForwardVector) * UU_TO_M;
    float Vy = FVector::DotProduct(WheelVelocity, W.WheelRightVector) * UU_TO_M;
    if (FMath::Abs(Vx) < 0.01f) return 0.f;
    return FMath::RadiansToDegrees(FMath::Atan2(Vy, Vx));
}

float UVehicleMovementComponent::ComputeLongSlipRatio(const FVehicleWheelState& W, const FVector& WheelVelocity) const
{
    float LongSpeedMS = FVector::DotProduct(WheelVelocity, W.WheelForwardVector) * UU_TO_M;
    float WheelSurfMS = W.WheelAngularVelocity * W.WheelRadius;
    float Denom = FMath::Max(FMath::Abs(LongSpeedMS), 0.1f);
    return FMath::Clamp((WheelSurfMS - LongSpeedMS) / Denom, -1.f, 1.f);
}

float UVehicleMovementComponent::LongFrictionFromSlip(float SlipRatio) const
{
    // Simplified Pacejka Magic Formula shape
    return EngineConfig.MuPeak * FMath::Sin(1.65f * FMath::Atan(SlipRatio / 0.28f));
}

float UVehicleMovementComponent::LatFrictionFromAngle(const FVehicleWheelState& W, float SlipAngleDeg) const
{
    if (!W.LateralGripCurve.GetRichCurveConst())
        return 0.f;

    float Mu = W.LateralGripCurve.GetRichCurveConst()->Eval(FMath::Abs(SlipAngleDeg));
    return FMath::Sign(SlipAngleDeg) * Mu;
}

bool UVehicleMovementComponent::IsWheelDriven(const FVehicleWheelState& W) const
{
    switch (EngineConfig.DriveLayout)
    {
    case EDriveLayout::FrontWheelDrive: return W.bIsFrontWheel;
    case EDriveLayout::RearWheelDrive:  return !W.bIsFrontWheel;
    case EDriveLayout::AllWheelDrive:   return true;
    default:                            return false;
    }
}

float UVehicleMovementComponent::TotalDriveRatio() const
{
    if (!EngineConfig.GearRatios.IsValidIndex(CurrentGear))
        return 1.f;

    return EngineConfig.GearRatios[CurrentGear] * EngineConfig.DifferentialRatio;
}

float UVehicleMovementComponent::ComputeWheelResistanceTorque(const FVehicleWheelState& W) const
{
    if (!W.bIsGrounded) return 0.f;

    // Rolling resistance
    float Rolling = EngineConfig.RollingResistanceCoeff * W.NormalForce;

    // Aero drag split across driven wheels
    float AeroForce = 0.5f * AIR_DENSITY * EngineConfig.AeroDragCoeff * EngineConfig.FrontalArea * VehicleSpeed * VehicleSpeed;
    int32 NumDriven = 0;
    for (const FVehicleWheelState& W2 : Wheels)
    {
        if (IsWheelDriven(W2))
            ++NumDriven;
    }
    float AeroPerWheel = NumDriven > 0 ? AeroForce / NumDriven : 0.f;

    return (Rolling + AeroPerWheel) * W.WheelRadius;
}

float UVehicleMovementComponent::ComputeEngineBrakeTorque() const
{
    if (ThrottleInput > 0.1f || EngineRPM <= EngineConfig.IdleRPM)
        return 0.f;

    return (EngineRPM / EngineConfig.MaxRPM) * 300.f;
}

UE_ENABLE_OPTIMIZATION
