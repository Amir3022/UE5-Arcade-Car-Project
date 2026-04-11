// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"

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
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	//Disable Collision for all Wheel Mesh Components
	if (VehicleWheelComponents.Num())
	{
		for (UStaticMeshComponent* WheelMesh : VehicleWheelComponents)
		{
			if (WheelMesh)
			{
				WheelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//Disable Collision for all wheels
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Amir, Wheels not assigned yet!"));
	}

	//Lower the Center of mass for the main Vehicle Body for better stability
	VehicleBody->SetCenterOfMass(FVector(0.0f, 0.0f, -30.0f));
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

