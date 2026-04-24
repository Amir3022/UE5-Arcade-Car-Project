// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "CoreMinimal.h"
#include "ArcadeCarProj_5_7/Core/Data/VehicleTypes.h"
#include "VehicleInterface.generated.h"

UINTERFACE(BlueprintType)
class ARCADECARPROJ_5_7_API UVehicleInterface : public UInterface
{
	GENERATED_BODY()
};

class IVehicleInterface
{
	GENERATED_BODY()

public:
	virtual void SetTargetThrottleInput(float ThrottleValue) {}
	virtual void SetTargetSteeringValue(float InSteeringValue) {}
	virtual void SetUsingHandbrake(bool InUseHandbrake) {}
	virtual void SetCurrentTransmissionMode(ETransmissionMode InTransmissionMode) {}
	virtual void DoGearUpShift() {}
	virtual void DoGearDownShift() {}
};