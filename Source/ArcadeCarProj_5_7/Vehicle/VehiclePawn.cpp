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

	ForwardThrottleStrength = 10000.0f;
	ReverseThrottleStrength = 2000.0f;
	BrakingThrottleStrength = 500000;

	bIsGrounded = false;
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
	//Initialize Wheel Variables
	InitializeAttachedWheels();

	//Lower the Center of mass for the main Vehicle Body for better stability
	VehicleBody->SetCenterOfMass(FVector(0.0f, 0.0f, -60.0f));
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Check for Each wheel and vehicle grounding
	CheckGrounding();

	//Suspension forces at each wheel base
	ApplySuspensionForces();
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

void AVehiclePawn::ApplyThrottleForce(float ThrottleForce)
{
	if (VehicleBody && bIsGrounded)	//Vehicle must be grounded to apply throttle
	{
		if (ThrottleForce >= 0)	//Applying Forward Throttle
		{
			FVector ForwardForce = ThrottleForce * ForwardThrottleStrength * VehicleBody->GetForwardVector();
			VehicleBody->AddForceAtLocation(ForwardForce, VehicleBody->GetCenterOfMass());
		}
		else   //Applying Reverse Throttle
		{
			FVector ReverseForce;
			//Get Forward Velocity, if larger than a small maring apply braking power from reverse input
			if (FVector::DotProduct(VehicleBody->GetForwardVector(), VehicleBody->GetPhysicsLinearVelocity()) > 50.0f)
			{
				ReverseForce = ThrottleForce * BrakingThrottleStrength * VehicleBody->GetForwardVector();
			}
			else
			{
				ReverseForce = ThrottleForce * ReverseThrottleStrength * VehicleBody->GetForwardVector();
			}
			VehicleBody->AddForceAtLocation(ReverseForce, VehicleBody->GetCenterOfMass());
		}
	}
}

UE_ENABLE_OPTIMIZATION

