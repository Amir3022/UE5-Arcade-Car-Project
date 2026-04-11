// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Set this pawn to call AsyncPhysicTickActor() each phyiscs calculation update
	bAsyncPhysicsTickEnabled = true;

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
	
	//Initialize Wheel Variables
	InitializeAttachedWheels();

	//Lower the Center of mass for the main Vehicle Body for better stability
	VehicleBody->SetCenterOfMass(FVector(0.0f, 0.0f, -30.0f));
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Called at every update the physics thread
void AVehiclePawn::AsyncPhysicsTickActor(float DeltaTime, float SimTime)
{

}

//Initialize variables related to wheels using Attached Wheels Mesh Components set in the blueprint
void AVehiclePawn::InitializeAttachedWheels()
{
	WheelsLocalLocations.Empty();

	//Disable Collision for all Wheel Mesh Components
	if (VehicleWheelComponents.Num())
	{
		for (UStaticMeshComponent* WheelMesh : VehicleWheelComponents)
		{
			if (WheelMesh)
			{
				WheelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//Disable Collision for current Wheel
				WheelsLocalLocations.Add(WheelMesh->GetRelativeLocation());	//Get wheel relative location to body, add it to array
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Amir, Wheels not assigned yet!"));
	}
}

