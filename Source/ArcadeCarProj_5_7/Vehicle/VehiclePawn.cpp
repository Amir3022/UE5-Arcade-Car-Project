// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"

UE_DISABLE_OPTIMIZATION

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Vehicle Components
	VehicleBody = CreateDefaultSubobject<UStaticMeshComponent>(FName("VehicleBody"));
	VehicleBody->SetSimulatePhysics(true);	//Enable Simulation Physics for the Main vehicle Body to be enabled
	VehicleBody->SetCollisionProfileName(FName("PhysicsActor"));	//Set the Collision channel of the main body to be Physics Actor
	RootComponent = VehicleBody;	//Set the VehicleBody as the RootComponent of the pawn

	//Initializing Variables
	GroundCheckTolerance = 5.0f;

	SpringRestLength = 30.0f;
	SpringStiffness = 50000.0f;
	SpringDamping = 7000.0f;

	DriveTrainType = EDriveTrainType::FWD;
	ThrottleValueChangeSpeed = 5.0f;
	MaxEngineTorque = 2800.0f;
	RevRate = 12.0f;
	IdleRPM = 900.0f;
	RedlineRPM = 7200.0f;
	FinalDriveTrainRatio = 3.9f;
	DriveTrainEfficiency = 0.9f;
	StandardWheelRadius = 50.f;
	MaxForwardSpeed = 200.0f;
	MaxReverseSpeed = 50.0f;

	MaxSteeringAngleDegrees = 30.0f;
	SteeringAngularSpeedFrac = 5.0f;

	bIsGrounded = false;
	CurrentTransmissionMode = ETransmissionMode::Neutral;
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
	//Initialize Wheel Variables
	InitializeAttachedWheels();

	//Lower the Center of mass for the main Vehicle Body for better stability
	VehicleBody->SetCenterOfMass(FVector(0.0f, 0.0f, -55.0f));	//TODO - Make this calculated instead of being a fixed value
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UE_LOG(LogTemp, Warning, TEXT("Amir"));

	//Check for Each wheel and vehicle grounding
	CheckGrounding();

	//Suspension forces at each wheel base
	ApplySuspensionForces();

	//Update Throttle Value, Rev Engine RPM on the updated Current Throttle Value
	UpdateThrottle(DeltaTime);

	//Update Current Engine RPM Based on the Current Transmission Mode
	UpdateEngineRPM(DeltaTime);

	//Use EngineRPM to apply Torque to the wheels and move the vehicle
	ApplyEngineTorque();

	//Update Steering Value, Apply Steering Force based on the Updated Steering value
	UpdateAndApplySteering(DeltaTime);

	float ForwardSpeed = GetCurrentForwardSpeedKMH();
	UE_LOG(LogTemp, Warning, TEXT("Amir, Current Transmission Mode: %i"), (int32)CurrentTransmissionMode);
	UE_LOG(LogTemp, Warning, TEXT("Amir, Current Speed: %f KMH"), ForwardSpeed);
	UE_LOG(LogTemp, Warning, TEXT("Amir, Current RPM: %f"), GetCurrentRPM());
	DrawDebugSphere(GetWorld(), VehicleBody->GetCenterOfMass(), 10.0f, 16, FColor::Blue, false, -1.0f, 10);
}

//Called at every update the physics thread
void AVehiclePawn::AsyncPhysicsTickActor(float DeltaTime, float SimTime)
{
	Super::AsyncPhysicsTickActor(DeltaTime, SimTime);
}

//Initialize variables related to wheels using Attached Wheels Mesh Components set in the blueprint
void AVehiclePawn::InitializeAttachedWheels()
{
	WheelStates.Empty();

	//Disable Collision for all Wheel Mesh Components
	if (VehicleWheelComponents.Num())
	{
		for (UStaticMeshComponent* WheelMesh : VehicleWheelComponents)
		{
			if (WheelMesh)
			{
				FWheelState CurrentWheelState;
				CurrentWheelState.OffsetTransform = WheelMesh->GetRelativeTransform();//Add Wheel Initial Rel Transform, to be used for all calculations
				CurrentWheelState.WheelName = WheelMesh->GetName();
				//Based on the Wheel Mesh name, determine the current wheel type - TODO: Fix this by having the wheel be a separate component
				if (WheelMesh->GetName().Contains("FL") || WheelMesh->GetName().Contains("FR"))
				{
					CurrentWheelState.WheelType = EWheelType::FrontWheel;
					if (DriveTrainType == EDriveTrainType::FWD || DriveTrainType == EDriveTrainType::AWD)
						CurrentWheelState.bDrivingWheel = true;
				}
				else
				{
					CurrentWheelState.WheelType = EWheelType::RearWheel;
					if (DriveTrainType == EDriveTrainType::RWD || DriveTrainType == EDriveTrainType::AWD)
						CurrentWheelState.bDrivingWheel = true;
				}

				WheelStates.Add(CurrentWheelState);	

				WheelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//Disable Collision for current Wheel
				WheelMesh->SetGenerateOverlapEvents(false);	//Disable overlap detection for the wheel mesh components
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Amir, Wheels not assigned yet!"));
	}
}

void AVehiclePawn::CheckGrounding()
{
	if (VehicleBody)
	{
		int32 NumOfWheelsGrounded = 0;

		//Ray Trace from each wheel origin, determine wheel state
		for (FWheelState& CurrentWheelState : WheelStates)
		{
			FTransform CurrentWheelTransform = CurrentWheelState.OffsetTransform * VehicleBody->GetComponentTransform();
			FVector StartLocation = CurrentWheelTransform.GetLocation();
			FVector EndLocation = StartLocation - (SpringRestLength + GroundCheckTolerance) * VehicleBody->GetUpVector();
			FHitResult WheelTraceResult;
			FCollisionQueryParams WheelTraceParams("WheelTrace");
			WheelTraceParams.AddIgnoredActor(this);
			if (GetWorld()->LineTraceSingleByChannel(WheelTraceResult, StartLocation, EndLocation, ECC_Visibility, WheelTraceParams))
			{
				CurrentWheelState.bGrounded = true;
				CurrentWheelState.DistanceToGround = WheelTraceResult.Distance;
				CurrentWheelState.ContactPointNormal = WheelTraceResult.ImpactNormal;
				NumOfWheelsGrounded++;
			}
			else
			{
				CurrentWheelState.bGrounded = false;
				CurrentWheelState.DistanceToGround = FLT_MAX;
				CurrentWheelState.ContactPointNormal = FVector::ZeroVector;
			}
		}

		bIsGrounded = NumOfWheelsGrounded >= (WheelStates.Num()) / 2; //Consider the vehicle grounded if more than half the wheels are grounded
	}
}

void AVehiclePawn::ApplySuspensionForces()
{
	//If grounded, apply antigravity and Suspension
	if (VehicleBody)
	{
		//Apply Suspension Spring Force at Each wheel base
		for (const FWheelState& CurrentWheelState : WheelStates)
		{
			if (CurrentWheelState.bGrounded)
			{
				FTransform CurrentWheelTransform = CurrentWheelState.OffsetTransform * VehicleBody->GetComponentTransform();

				float Compression = SpringRestLength - CurrentWheelState.DistanceToGround;
				float SpringForce = SpringStiffness * Compression;

				FVector WheelPhysVel = VehicleBody->GetPhysicsLinearVelocityAtPoint(CurrentWheelTransform.GetLocation());
				float WheelPhysVerticalVel = VehicleBody->GetUpVector().Dot(WheelPhysVel);
				float DampingForce = SpringDamping * WheelPhysVerticalVel;
				
				FVector TotalAppliedSpringForce = VehicleBody->GetUpVector() * (SpringForce - DampingForce);

				VehicleBody->AddForceAtLocation(TotalAppliedSpringForce, CurrentWheelTransform.GetLocation());

				DrawDebugLine(GetWorld(), CurrentWheelTransform.GetLocation(), CurrentWheelTransform.GetLocation() + TotalAppliedSpringForce * 0.05f, FColor::Red, false);
			}
		}
	}
}

void AVehiclePawn::UpdateThrottle(float deltaSeconds)
{
	if (VehicleBody)
	{
		//Update the Current Throttle Value base on ThrottleValueChangeSpeed speed and TargetThrottleValue
		bool IncreasingInForwardDirection = CurrentThrottleValue <= TargetThrottleValue;
		if (CurrentThrottleValue != TargetThrottleValue)
		{
			CurrentThrottleValue += (IncreasingInForwardDirection ? 1.0f : -1.0f) * deltaSeconds * ThrottleValueChangeSpeed;
			if ((IncreasingInForwardDirection && (CurrentThrottleValue > TargetThrottleValue)) || (!IncreasingInForwardDirection && (CurrentThrottleValue < TargetThrottleValue)))
				CurrentThrottleValue = TargetThrottleValue;
		}
		UE_LOG(LogTemp, Warning, TEXT("Amir, Current Throttle Value: %f"), CurrentThrottleValue);
	}
}

void AVehiclePawn::UpdateEngineRPM(float deltaSeconds)
{
	if (VehicleBody)
	{
		switch (CurrentTransmissionMode)
		{
		case ETransmissionMode::Neutral:	//On Neutral, the engine should Rev up with Throttle Input
		{
			CurrentRPM = FMath::Clamp(CurrentRPM + (CurrentThrottleValue * RevRate - EngineFrictionRate) * deltaSeconds, IdleRPM, RedlineRPM);
			break;
		}
		case ETransmissionMode::Drive:	//On Drive, the RPM is dectated by the current wheel rotation speed and the rotation transmitted throught Transmission and Differential
		{
			if (GearRatios.IsValidIndex(CurrentGearIndex))
			{
				float CurrentSpeedCMS = GetCurrentForwardSpeedKMH() * (5.0f / 18.0f) * 100.0f;
				float WheelCircumference = 2.0f * UE_PI * StandardWheelRadius;
				float WheelRPM = (CurrentSpeedCMS / WheelCircumference) * 60.0f;
				CurrentRPM = FMath::Clamp(WheelRPM * FinalDriveTrainRatio * GearRatios[CurrentGearIndex], IdleRPM, RedlineRPM);
			}
			break;
		}
		}
	}
}

void AVehiclePawn::ApplyEngineTorque()
{
	if (VehicleBody && IsTransmissionEngaged())	//Vehicle must be grounded to apply throttle
	{
		float CurrentSpeed = GetCurrentForwardSpeedKMH();
		//Use Current RPM to Get Engine Torque from Curve
		float CurrentEngineTorque = RPMTorqueCurve.GetRichCurve()->Eval(GetCurrentRPM()) * FMath::Abs(CurrentThrottleValue);
		//Get Wheel Axel Torque by Multiplying the Engine Torque by the Current Gear Ratio and Final Drive Train Ration
		float WheelsAxelTorque = CurrentEngineTorque * GearRatios[CurrentGearIndex] * FinalDriveTrainRatio * DriveTrainEfficiency;
		//Get the Totatl Force applied by the Engine to the wheels
		float LinearEngineForce = WheelsAxelTorque / (StandardWheelRadius / 100.0f) * 100.0f;
		UE_LOG(LogTemp, Warning, TEXT("Amir, Applied Linear Force: %f, Engine Torque: %f, Wheel Axel Torque: %f"), LinearEngineForce, CurrentEngineTorque, WheelsAxelTorque);

		if (CurrentThrottleValue >= 0)	//Apply force if only we are below the Max Forward Speed
		{
			if (CurrentSpeed < MaxForwardSpeed)
			{
				float ForwardForce = LinearEngineForce;
				DistributeForceToDrivingWheels(ForwardForce);
			}
		}
		else   //Apply force if only we are below the Max Reverse Speed
		{
			if (CurrentSpeed > (-1 * MaxReverseSpeed))
			{
				float ReverseForce = LinearEngineForce;
				DistributeForceToDrivingWheels(ReverseForce);
			}
		}
	}
}

void AVehiclePawn::DistributeForceToDrivingWheels(float ThrottleForce)
{
	if (VehicleBody)
	{
		//Get all Driving Wheels
		TArray<FWheelState> DrivingWheels;
		for (FWheelState CurrentWheelState : WheelStates)
		{
			if (CurrentWheelState.bDrivingWheel)
				DrivingWheels.Add(CurrentWheelState);
		}

		//Distribute force on all driving wheels
		float ForcePerWheel = ThrottleForce / DrivingWheels.Num();

		//Apply force at each wheel location
		for (const FWheelState& DrivingWheel : DrivingWheels)
		{
			if (DrivingWheel.bGrounded)
			{
				FVector DrivingWheelLocation = (DrivingWheel.OffsetTransform * VehicleBody->GetComponentTransform()).GetLocation();
				//Get forward movement direction based on wheel contact normal on ground
				FVector WheelForwardVector = FVector::CrossProduct(VehicleBody->GetRightVector(),  DrivingWheel.ContactPointNormal);
				FVector ActingForce = WheelForwardVector * ForcePerWheel;
				VehicleBody->AddForceAtLocation(ActingForce, DrivingWheelLocation);
			}
		}
	}
}

void AVehiclePawn::UpdateAndApplySteering(float deltaSeconds)
{
	if (VehicleBody)
	{
		//Update the Current Steering Value base on SteeringAngular speed and TargetSteeringValue
		bool IncreasingInRightDirection = CurrentSteeringValue <= TargetSteeringValue;
		if (CurrentSteeringValue != TargetSteeringValue)
		{
			CurrentSteeringValue += (IncreasingInRightDirection ? 1.0f : -1.0f) * deltaSeconds * SteeringAngularSpeedFrac;
			if ((IncreasingInRightDirection && (CurrentSteeringValue > TargetSteeringValue)) || (!IncreasingInRightDirection && (CurrentSteeringValue < TargetSteeringValue)))
				CurrentSteeringValue = TargetSteeringValue;
		}

		UE_LOG(LogTemp, Warning, TEXT("Amir, Current Steering Value: %f"), CurrentSteeringValue);

		//Apply steering wheel visuals by rotating the front wheels with steering angle
		float SteeringAngle = CurrentSteeringValue * MaxSteeringAngleDegrees;
		for (int32 i = 0; i < VehicleWheelComponents.Num(); i++)
		{
			if (VehicleWheelComponents[i] && WheelStates[i].WheelType == EWheelType::FrontWheel)
			{
				FRotator NewWheelLocalRotation = VehicleWheelComponents[i]->GetRelativeRotation();
				NewWheelLocalRotation.Yaw = SteeringAngle;
				VehicleWheelComponents[i]->SetRelativeRotation(NewWheelLocalRotation);
				//Set Steering Angle in the wheel state of front wheels
				WheelStates[i].SteeringAngle = SteeringAngle;
			}
		}

		//Apply steering if the Vehicle is Grounded
		if (bIsGrounded)
		{
			//Get Current Vehicle Speed in KMH
			float CurrentSpeedFactor = GetCurrentForwardSpeedKMH() / (GetCurrentForwardSpeedKMH() >= 0.0f ? MaxForwardSpeed : MaxReverseSpeed);
			//Apply Steering at each wheel location
			for (const FWheelState& CurrentWheel : WheelStates)
			{
				if (CurrentWheel.bGrounded)
				{
					FVector CurrentWheelLocation = (CurrentWheel.OffsetTransform * VehicleBody->GetComponentTransform()).GetLocation();
					//Get forward movement direction based on wheel contact normal on ground
					FVector WheelForwardVector = FVector::CrossProduct(VehicleBody->GetRightVector(), CurrentWheel.ContactPointNormal);
					///Rotate the Forward vector around contact normal with Steering angle
					WheelForwardVector = WheelForwardVector.RotateAngleAxis(CurrentWheel.SteeringAngle, CurrentWheel.ContactPointNormal);
					//Get Right Vector Based on new forward vector and contact point normal
					FVector WheelRightVector = FVector::CrossProduct(CurrentWheel.ContactPointNormal, WheelForwardVector);
					//Get Wheel Velocity
					FVector WheelVelocity = VehicleBody->GetPhysicsLinearVelocityAtPoint(CurrentWheelLocation);
					//Get Wheel velocity in the direction of the steering(Wheel Calculated right vector)
					float SteeringVelocity = WheelVelocity.Dot(WheelRightVector);
					float DesiredVelocityChange = -SteeringVelocity;
					if (CurrentWheel.WheelType == EWheelType::FrontWheel)
						DesiredVelocityChange *= FrontSpeedGripCurve.GetRichCurve()->Eval(CurrentSpeedFactor);
					else if(CurrentWheel.WheelType == EWheelType::RearWheel)
						DesiredVelocityChange *= RearSpeedGripCurve.GetRichCurve()->Eval(CurrentSpeedFactor);
					//Force to be applied at wheel location to do steering
					FVector ForceToApply = DesiredVelocityChange / deltaSeconds * VehicleBody->GetMass() / WheelStates.Num() * WheelRightVector;
					//Apply Force to Vehicle body at Wheel Location
					VehicleBody->AddForceAtLocation(ForceToApply, CurrentWheelLocation);

					DrawDebugLine(GetWorld(), CurrentWheelLocation, CurrentWheelLocation + WheelForwardVector * 100.0f, FColor::Blue, false, -1.0f, 10);
					DrawDebugLine(GetWorld(), CurrentWheelLocation, CurrentWheelLocation + WheelRightVector * 100.0f, FColor::Green, false, -1.0f, 10);
				}
			}
		}
	}
}

void AVehiclePawn::SetTargetThrottleInput(float InThrottleForce)
{
	if (VehicleBody)
	{
		TargetThrottleValue = InThrottleForce;
	}
}

void AVehiclePawn::SetTargetSteeringValue(float InSteeringValue)
{
	if (VehicleBody)
	{
		TargetSteeringValue = InSteeringValue;
	}
}

float AVehiclePawn::GetCurrentForwardSpeedKMH()
{
	if (!VehicleBody)
		return 0.0f;

	return FVector::DotProduct(VehicleBody->GetForwardVector(), VehicleBody->GetPhysicsLinearVelocity()) / 100.0f * 18.0f / 5.0f;	//Get the Current Speed in KMH
}

float AVehiclePawn::GetCurrentRPM()
{
	if (!VehicleBody)
		return 0.0f;

	return CurrentRPM;
}

bool AVehiclePawn::IsTransmissionEngaged()
{
	return bIsGrounded && GearRatios.IsValidIndex(CurrentGearIndex) && (CurrentTransmissionMode == ETransmissionMode::Drive || CurrentTransmissionMode == ETransmissionMode::Reverse);
}

UE_ENABLE_OPTIMIZATION

