// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoidConstants.h"

class BOIDS_API FFlockMath
{
public:
	static FVector3f CalculateAlignment(const uint32 Index, const TArray<FVector3f>& Velocities, TArray<uint32>& InNeighbors, const float Strength);
	static FVector3f CalculateCohesion(uint32 Index, const TArray<FVector3f>& Positions, TArray<uint32>& InNeighbors, float Strength);
	static FVector3f CalculateSeparation(const uint32 Index, const TArray<FVector3f>& Positions, const TArray<uint32>& InNeighbors, const float Strength);
	static FVector3f CalculateTarget(FVector3f Position, FVector3f Target, float Strength, const FBoidMovement& Settings);
};
