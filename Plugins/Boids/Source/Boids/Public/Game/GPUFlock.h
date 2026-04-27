// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Boids/Public/GPU/Structs/GPUStructs.h"
#include "Core/BoidConstants.h"
#include "GameFramework/Actor.h"
#include "GPUFlock.generated.h"

class UGPUDrawComponent;

USTRUCT()
struct FFlockSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	float SpeedModifier;

	UPROPERTY(EditAnywhere)
	FStrengths Strengths;

	UPROPERTY(EditAnywhere)
	FBoidMovement Movement;
	
	UPROPERTY(EditAnywhere)
	FHashSettings Hash;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = 1))
	int32 BoidCount;
};

UCLASS()
class BOIDS_API AGPUFlock : public AActor
{
	GENERATED_BODY()

public:
	AGPUFlock();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void GameInit();
	virtual void RenderInit();
	virtual void GameTick(float DeltaTime);
	virtual void RenderTick();
	virtual void Dispose();

	virtual void InitializeBuffers(FRDGBuilder& GraphBuilder);
	virtual void RebuildHashes(FRDGBuilder& GraphBuilder);
	virtual void RebuildOffsetBuffer(FRDGBuilder& GraphBuilder);
	virtual void UpdateBoidHeading(FRDGBuilder& GraphBuilder);

	UPROPERTY(EditAnywhere, Category="01. SETTINGS")
	FFlockSettings Settings;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UGPUDrawComponent> DrawComponent;

private:
	FDoubleBuffer PositionBuffer = FDoubleBuffer();
	FDoubleBuffer VelocityBuffer = FDoubleBuffer();
	
	FBoidsRDGData BoidsRDGData = FBoidsRDGData(1);

	FUAVBuffer HashBuffer = FUAVBuffer();
	FUAVBuffer BoidIndexBuffer = FUAVBuffer();
	FUAVBuffer CellOffsetBuffer = FUAVBuffer();

	TRefCountPtr<FRDGPooledBuffer> RenderPositionBuffer;
	TRefCountPtr<FRDGPooledBuffer> RenderVelocityBuffer;

	float LastDeltaTime;
};
