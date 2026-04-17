// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FTestValues
{
	float FPS = 0;
	float UpdateTime = 0;
};

class BOIDS_API TestInspector : public AActor
{
public:
	void SaveValues();
private:
	void SaveTotal();
	void SaveAvg();
	
	uint32 TotalFrames = 0;
	FTestValues Total;
	FTestValues Avg;
};
