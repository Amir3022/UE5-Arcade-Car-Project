// Fill out your copyright notice in the Description page of Project Settings.


#include "RealisticVehiclePawn.h"

// Sets default values
ARealisticVehiclePawn::ARealisticVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initializing Components
	ChassisComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("VehicleChassis"));
	RootComponent = ChassisComp;
	VehicleMovementComp = CreateDefaultSubobject<UVehicleMovementComponent>(FName("CarMovementComp"));

	//Initializing Input Variables
	ThrottleValueChangeSpeed = 5.0f;
	BrakeValueChangeSpeed = 10.0f;
	SteeringAngularSpeedFrac = 5.0f;
}

// Called when the game starts or when spawned
void ARealisticVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARealisticVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ChassisComp && VehicleMovementComp)
	{
		UpdateThrottleInput(DeltaTime);
		UpdateBrakingInput(DeltaTime);
		UpdateSteeringInput(DeltaTime);
	}
}

void ARealisticVehiclePawn::DoGearUpShift()
{
	if (VehicleMovementComp)
	{
		int32 MaxGearIndex = VehicleMovementComp->GetEngineConfig().GearRatios.Num() - 1;
		VehicleMovementComp->SetCurrentGear(FMath::Clamp(VehicleMovementComp->GetCurrentGear() + 1, 0, MaxGearIndex));
	}
}

void ARealisticVehiclePawn::DoGearDownShift()
{
	if (VehicleMovementComp)
	{
		int32 MaxGearIndex = VehicleMovementComp->GetEngineConfig().GearRatios.Num() - 1;
		VehicleMovementComp->SetCurrentGear(FMath::Clamp(VehicleMovementComp->GetCurrentGear() - 1, 0, MaxGearIndex));
	}
}

void ARealisticVehiclePawn::UpdateThrottleInput(float deltaSeconds)
{
	if (VehicleMovementComp)
	{
		//Update the Current Throttle Value base on ThrottleValueChangeSpeed speed and TargetThrottleValue
		bool IncreasingInForwardDirection = CurrentThrottleValue <= TargetThrottleValue;
		if (CurrentThrottleValue != TargetThrottleValue)
		{
			CurrentThrottleValue += (IncreasingInForwardDirection ? 1.0f : -1.0f) * deltaSeconds * ThrottleValueChangeSpeed;
			if ((IncreasingInForwardDirection && (CurrentThrottleValue > TargetThrottleValue)) || (!IncreasingInForwardDirection && (CurrentThrottleValue < TargetThrottleValue)))
				CurrentThrottleValue = TargetThrottleValue;
		}
		//Set the Throttle Input Value in the Movement Comp
		UE_LOG(LogTemp, Warning, TEXT("Amir, Throttle: %f"), CurrentThrottleValue);
		VehicleMovementComp->SetThrottleInput(CurrentThrottleValue);
	}
}

void ARealisticVehiclePawn::UpdateBrakingInput(float deltaSeconds)
{
	if (VehicleMovementComp)
	{
		//Update the Current Brake Value base on BrakeValueChangeSpeed speed and TargetBrakeValue
		bool IncreasingInForwardDirection = CurrentBrakeValue <= TargetBrakeValue;
		if (CurrentBrakeValue != TargetBrakeValue)
		{
			CurrentBrakeValue += (IncreasingInForwardDirection ? 1.0f : -1.0f) * deltaSeconds * BrakeValueChangeSpeed;
			if ((IncreasingInForwardDirection && (CurrentBrakeValue > TargetBrakeValue)) || (!IncreasingInForwardDirection && (CurrentBrakeValue < TargetBrakeValue)))
				CurrentBrakeValue = TargetBrakeValue;
		}
		//Set the Brake Input Value in the Movement Comp
		UE_LOG(LogTemp, Warning, TEXT("Amir, Brake: %f"), CurrentBrakeValue);
		VehicleMovementComp->SetBrakeInput(CurrentBrakeValue);
	}
}

void ARealisticVehiclePawn::UpdateSteeringInput(float deltaSeconds)
{
	if (VehicleMovementComp)
	{
		//Update the Current Steering Value base on SteeringAngular speed and TargetSteeringValue
		bool IncreasingInRightDirection = CurrentSteeringValue <= TargetSteeringValue;
		if (CurrentSteeringValue != TargetSteeringValue)
		{
			CurrentSteeringValue += (IncreasingInRightDirection ? 1.0f : -1.0f) * deltaSeconds * SteeringAngularSpeedFrac;
			if ((IncreasingInRightDirection && (CurrentSteeringValue > TargetSteeringValue)) || (!IncreasingInRightDirection && (CurrentSteeringValue < TargetSteeringValue)))
				CurrentSteeringValue = TargetSteeringValue;
		}
	}
	//Set the Steering Input value in MovementComp
	UE_LOG(LogTemp, Warning, TEXT("Amir, Steering: %f"), CurrentSteeringValue);
	VehicleMovementComp->SetSteeringInput(CurrentSteeringValue);
}

