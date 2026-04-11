// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehiclePawn.generated.h"

struct FWheelState
{
	FString WheelName;
	FTransform OffsetTransform;
	bool bGrounded;
	float DistanceToGround;
	FVector ContactPointNormal;

	FWheelState()
	{
		WheelName = "";
		OffsetTransform = FTransform();
		bGrounded = false;
		DistanceToGround = FLT_MAX;
		ContactPointNormal = FVector::ZeroVector;
	}
};

UCLASS()
class ARCADECARPROJ_5_7_API AVehiclePawn : public APawn
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
	void UpdateAndApplySteering(float deltaSeconds);

	float GetCurrentForwardSpeedKMH();
	
public:
	//Input Related Functions
	void ApplyThrottleForce(float ThrottleValue);
	void SetTargetSteeringValue(float InSteeringValue);

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
	float ForwardThrottleStrength;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float ReverseThrottleStrength;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float BrakingThrottleStrength;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float MaxForwardSpeed;
	UPROPERTY(EditAnywhere, Category = "Throttle")
	float MaxReverseSpeed;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float MaxSteeringTorque;
	UPROPERTY(EditAnywhere, Category = "Steering")
	float SteeringAngularSpeedFrac;
	UPROPERTY(EditAnywhere, Category = "Steering")
	FRuntimeFloatCurve SteeringSpeedTorqueCurve;

private:
	//Vehicle Movement Variables
	TArray<FWheelState> WheelStates;
	bool bIsGrounded;
	float TargetSteeringValue;
	float CurrentSteeringValue;
};
