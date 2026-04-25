// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "VehicleTypes.generated.h"

UENUM(BlueprintType)
enum EDriveTrainType : uint8
{
	FWD,
	RWD,
	AWD,
};

enum EWheelType : uint8
{
	None,
	FrontWheel,
	RearWheel,
};

enum ETransmissionMode
{
	Neutral,
	Drive,
	Reverse,
	Park,
};

struct FWheelState
{
	FString WheelName;
	FTransform OffsetTransform;
	bool bGrounded;
	float DistanceToGround;
	FVector ContactPointNormal;
	float SteeringAngle;
	EWheelType WheelType;
	bool bDrivingWheel;

	FWheelState()
	{
		WheelName = "";
		OffsetTransform = FTransform();
		bGrounded = false;
		DistanceToGround = FLT_MAX;
		ContactPointNormal = FVector::ZeroVector;
		SteeringAngle = 0.0f;
		WheelType = EWheelType::None;
		bDrivingWheel = false;
	}
};