#pragma once

#include "CoreMinimal.h"
#include "BoidConstants.generated.h"

class UNiagaraSystem;
class ABoidActor;
constexpr float GOLDEN_RATIO = 1.618;

static FIntVector ThreadGroupSize(const int NumElements)
{
	constexpr int ThreadCount = 256;
	const int Count = ((NumElements - 1) / ThreadCount) + 1;
	return FIntVector(Count, 1, 1);
}

USTRUCT()
struct FBoidsSOA
{
	GENERATED_BODY()
public:
	TArray<FVector3f> Positions;
	TArray<FVector3f> Velocities;
};

USTRUCT()
struct FStrengths
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,  meta = (ClampMin = 0, ClampMax = 2))
	float Alignment = .1f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 2))
	float Cohesion = .1f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 2))
	float Separation = .1f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 2))
	float Target = .1f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 2))
	float PlayerAvoidance = .1f;
};

USTRUCT()
struct FBoidMovement
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float BaseSpeed = 10.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float SeparationDistance = 3.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float AvoidanceDistance = 3.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float MinDistanceFromTarget = 200.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float MaxDistanceFromTarget = 200.f;
};

USTRUCT()
struct FHashSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = 1))
	float CellSize = 100.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.1, ClampMax = 3.f))
	float SearchModifier = 1.f;
};

USTRUCT()
struct FCPUSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABoidActor> BoidBP;
};

USTRUCT()
struct FGPUSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;
};


USTRUCT()
struct FBoidSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	uint32 BoidAmount = 1000;
	
	UPROPERTY(EditAnywhere)
	FStrengths Strengths;
	
	UPROPERTY(EditAnywhere)
	FBoidMovement Movement;

	UPROPERTY(EditAnywhere)
	FHashSettings Hash;
};

USTRUCT()
struct FTestSettings : public FBoidSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FCPUSettings CPU;

	UPROPERTY(EditAnywhere)
	FGPUSettings GPU;

	UPROPERTY(EditAnywhere)
	FVector3f Target;
};

UENUM()
enum class EFlockType : uint8
{
	SINGLE = 0,
	MULTI = 1,
};