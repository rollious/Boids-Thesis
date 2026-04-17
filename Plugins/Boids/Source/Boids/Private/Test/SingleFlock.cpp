// Fill out your copyright notice in the Description page of Project Settings.

#include "Test/SingleFlock.h"

#include "Core/BoidActor.h"
#include "Core/FlockMath.h"
#include "Core/SpatialHash.h"

// Sets default values
USingleFlock::USingleFlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;
}

void USingleFlock::InitFlock(const uint32 Amount, const FTestSettings& Settings)
{
	Super::InitFlock(Amount, Settings);
}

void USingleFlock::Work(float DeltaSeconds, const FTestSettings& Settings)
{
	Grid->Single_UpdateGrid(BoidSOA.Positions, Settings.Hash.CellSize);
	const int32 Num = BoidActors.Num();
	for (int32 i = 0; i < Num; i++) // Transform data in bulk
	{
		// Process data in bulk
		TArray<uint32> Neighbors;
		Grid->QueryNeighbors(BoidSOA.Positions[i], Neighbors, Settings.Hash.CellSize);
		FVector3f Acceleration =
				FFlockMath::CalculateAlignment(i, BoidSOA.Velocities, Neighbors, Settings.Strengths.Alignment)
			+ 	FFlockMath::CalculateCohesion(i, BoidSOA.Positions, Neighbors, Settings.Strengths.Cohesion)
			+	FFlockMath::CalculateSeparation(i, BoidSOA.Positions, Neighbors, Settings.Strengths.Separation)
			+	FFlockMath::CalculateTarget(BoidSOA.Positions[i], Settings.Target, Settings.Strengths.Target, Settings.Movement);
		
		BoidSOA.Velocities[i] += Acceleration * DeltaSeconds;
		BoidSOA.Velocities[i] = BoidSOA.Velocities[i].GetClampedToSize(Settings.Movement.BaseSpeed * 0.2, Settings.Movement.BaseSpeed);
	}
	
	for (int i = 0; i < BoidActors.Num(); i++) // Update actors in bulk
	{
		BoidSOA.Positions[i] += BoidSOA.Velocities[i] * DeltaSeconds;
		
		// Modify actors in bulk
		BoidActors[i]->SetActorLocation(FVector(BoidSOA.Positions[i]));
		BoidActors[i]->SetActorRotation(FQuat(BoidSOA.Velocities[i].ToOrientationQuat()));
	}
}
