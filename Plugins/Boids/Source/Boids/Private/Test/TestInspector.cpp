// Fill out your copyright notice in the Description page of Project Settings.

#include "Test/TestInspector.h"

void TestInspector::SaveValues()
{
	TotalFrames++;
	SaveTotal();
	SaveAvg();
}

void TestInspector::SaveTotal()
{
	float Delta = GetWorld()->DeltaRealTimeSeconds;

	Total.FPS += Delta;
	Total.UpdateTime += Delta;
}

void TestInspector::SaveAvg()
{
	Avg.UpdateTime = Total.UpdateTime / TotalFrames;
	Avg.FPS = TotalFrames / Total.FPS;
	GEngine->AddOnScreenDebugMessage(
		-1,
		-1,
		FColor::Green, FString::Printf(TEXT("FPS: %f, Update: %f"), Avg.FPS, Avg.UpdateTime));
}
