// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "PlayerInputConfig.generated.h"


UCLASS()
class ARCADECARPROJ_5_7_API UPlayerInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input Mapping")
	UInputAction* Throttle;
	UPROPERTY(EditDefaultsOnly, Category = "Input Mapping")
	UInputAction* Steer;
	UPROPERTY(EditDefaultsOnly, Category = "Input Mapping")
	UInputAction* HandBrake;
	UPROPERTY(EditDefaultsOnly, Category = "Input Mapping")
	UInputAction* Jump;
	UPROPERTY(EditDefaultsOnly, Category = "Input Mapping")
	UInputAction* FlipForward;
	UPROPERTY(EditDefaultsOnly, Category = "Input Mapping")
	UInputAction* FlipLateral;
};
