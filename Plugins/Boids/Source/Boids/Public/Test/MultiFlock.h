// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Flock.h"
#include "MultiFlock.generated.h"

UCLASS()
class BOIDS_API UMultiFlock : public UFlock
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UMultiFlock();

	virtual void InitFlock(uint32 Amount, const FTestSettings& Settings) override;
	virtual void Work(float DeltaSeconds, const FTestSettings& Settings) override;
};
