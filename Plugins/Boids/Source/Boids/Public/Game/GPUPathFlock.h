// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GPUFlock.h"
#include "GPUPathFlock.generated.h"

class USplineComponent;

USTRUCT()
struct FPath
{
	GENERATED_BODY()
public:
	UPROPERTY()
	float CurrentSplineTime = 0.f;

	UPROPERTY()
	float CurrentSpeed = 1.f;
	
	UPROPERTY()
	float BoostRemaining = 0.f;
	
	UPROPERTY()
	float BaseSpeed = 1.f; // Normal traversal speed

	UPROPERTY(EditAnywhere)
	float MinDistanceToTarget = 2.f;
	
	UPROPERTY(EditAnywhere)
	float BoostTime = 2.f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float BoostMultiplier = 2.f; // Speed during boost
};

UCLASS()
class BOIDS_API AGPUPathFlock : public AGPUFlock
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGPUPathFlock();

protected:
	
	void MoveTarget(float DeltaTime);
	
	UFUNCTION(CallInEditor, Category="00. FUNCS")
	void BoostMovement();

	virtual void GameInit() override;
	virtual void GameTick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category="01. SETTINGS")
	FPath Path;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineComponent> Spline;
};
