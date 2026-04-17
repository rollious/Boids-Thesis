// Fill out your copyright notice in the Description page of Project Settings.

#include "Test/Flock.h"
#include "Core/BoidActor.h"
#include "Core/SpatialHash.h"

// Sets default values
UFlock::UFlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;
	Grid = CreateDefaultSubobject<USpatialHash>("Grid");
}

void UFlock::InitFlock(const uint32 Amount, const FTestSettings& Settings)
{
	const FVector SpawnLocation = GetOwner()->GetActorLocation();
	const auto World = GetWorld();

	BoidSOA.Positions.SetNum(Amount);
	BoidSOA.Velocities.SetNum(Amount);
	
	FActorSpawnParameters BoidSpawnParams;
	BoidSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (uint32 i = 0; i < Amount; ++i)
	{
		FVector Location, Velocity;

		Location = FMath::VRand() * FMath::RandRange(0.f, 500.f);
		Location += SpawnLocation;
		Velocity = FMath::VRand() * FMath::FRandRange(-Settings.Movement.BaseSpeed, Settings.Movement.BaseSpeed);

		BoidSOA.Positions[i] = FVector3f(Location);
		BoidSOA.Velocities[i] = FVector3f(Velocity);

		ABoidActor* Boid = World->SpawnActor<ABoidActor>(Settings.CPU.BoidBP, Location,Velocity.ToOrientationRotator(), BoidSpawnParams);
		BoidActors.Add(Boid);
	}
}

void UFlock::Work(float DeltaSeconds, const FTestSettings& Settings)
{
}
