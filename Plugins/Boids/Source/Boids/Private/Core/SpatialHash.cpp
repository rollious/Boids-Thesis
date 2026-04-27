// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/SpatialHash.h"

// Sets default values for this component's properties
USpatialHash::USpatialHash()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}

void USpatialHash::Single_UpdateGrid(TArray<FVector3f>& Positions, float CellSize)
{
	for (int i = 0; i < Positions.Num(); i++)
	{
		FVector3f P = Positions[i];
		const FIntVector Cell = GridPosition(P, CellSize);
		int Hash = CreateHash(Cell);
		Grid.FindOrAdd(Hash).Add(i);
	}
}

// TODO: Doesn't work because TMap not thread safe.
void USpatialHash::Multi_UpdateGrid(TArray<FVector3f>& Positions, float CellSize)
{
	ParallelFor(Positions.Num(), [&](int32 i)
	{
		FVector3f P = Positions[i];
		const FIntVector Cell = GridPosition(P, CellSize);
		int Hash = CreateHash(Cell);
		Grid.FindOrAdd(Hash).Add(i);
	});
}

// TODO: Improve querying 
void USpatialHash::QueryNeighbors(const FVector3f& Position, TArray<uint32>& Neighbors, float CellSize)
{
	// GET SURROUNDING 24 CELLS AND THEIR CONTAINING BOIDS.
	Neighbors.Reset();
	const FIntVector CenterCell = GridPosition(Position, CellSize);

	// Search 3×3×3 = 27 surrounding cells
	for (int i = 0; i < 27; i++)
	{
		const FIntVector Neighbor = CenterCell + Offsets[i];
		const int Hash = CreateHash(Neighbor);

		// Check if cell exists in the grid
		if (Grid.Contains(Hash))
		{
			const TArray<uint32>& Indices = Grid[Hash];
			Neighbors.Append(Indices);
		}
	}
}
