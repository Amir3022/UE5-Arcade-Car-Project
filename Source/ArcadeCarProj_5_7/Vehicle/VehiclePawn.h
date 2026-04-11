// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehiclePawn.generated.h"

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
	void InitializeAttachedWheels();

protected:
	//Vehicle Pawn Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyComponents")
	UStaticMeshComponent* VehicleBody;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BodyComponents")
	TArray<UStaticMeshComponent*> VehicleWheelComponents;

private:
	TArray<FVector> WheelsLocalLocations;
};
