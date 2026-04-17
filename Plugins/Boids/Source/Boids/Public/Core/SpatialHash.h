// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpatialHash.generated.h"

struct FBoids;

static const FIntVector Offsets[27] =
{
	FIntVector(-1, -1, -1), FIntVector(0, -1, -1), FIntVector(1, -1, -1),
	FIntVector(-1,  0, -1), FIntVector(0,  0, -1), FIntVector(1,  0, -1),
	FIntVector(-1,  1, -1), FIntVector(0,  1, -1), FIntVector(1,  1, -1),

	FIntVector(-1, -1, 0),  FIntVector(0, -1, 0),  FIntVector(1, -1, 0),
	FIntVector(-1,  0, 0),  FIntVector(0,  0, 0),  FIntVector(1,  0, 0),
	FIntVector(-1,  1, 0),  FIntVector(0,  1, 0),  FIntVector(1,  1, 0),

	FIntVector(-1, -1, 1),  FIntVector(0, -1, 1),  FIntVector(1, -1, 1),
	FIntVector(-1,  0, 1),  FIntVector(0,  0, 1),  FIntVector(1,  0, 1),
	FIntVector(-1,  1, 1),  FIntVector(0,  1, 1),  FIntVector(1,  1, 1),
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BOIDS_API USpatialHash : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USpatialHash();

	void Single_UpdateGrid(TArray<FVector3f>& Positions, float CellSize);
	void Multi_UpdateGrid(TArray<FVector3f>& Positions, float CellSize);
	void QueryNeighbors(const FVector3f& Position, TArray<uint32>& Neighbors, float CellSize);

private:
	static int CreateHash(const FIntVector GridPosition) { return GridPosition.X * 73856093 ^ GridPosition.Y * 19349663 ^ GridPosition.Z * 83492791; }
	static FIntVector GridPosition(const FVector3f& Position, const float CellSize)
	{
		const FVector3f Scaled = Position / CellSize;
		return FIntVector(FMath::FloorToInt(Scaled.X), FMath::FloorToInt(Scaled.Y), FMath::FloorToInt(Scaled.Z));
	}

	// Hash, TArray<Boid Indices>
	TMap<int, TArray<uint32>> Grid;
};
