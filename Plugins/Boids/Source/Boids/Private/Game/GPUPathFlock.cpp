// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GPUPathFlock.h"
#include "Components/SplineComponent.h"


// Sets default values
AGPUPathFlock::AGPUPathFlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AGPUPathFlock::BoostMovement()
{
	Path.BoostRemaining = Path.BoostTime;
}

void AGPUPathFlock::GameInit()
{
	Super::GameInit();
	Target->SetWorldLocation(Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World));
}

void AGPUPathFlock::GameTick(float DeltaTime)
{
	MoveTarget(DeltaTime);
	Super::GameTick(DeltaTime);
}

void AGPUPathFlock::MoveTarget(float DeltaTime)
{
	//if (GetClosestPlayerDistance() <= Path.MinDistanceToTarget) BoostMovement();

	float TargetSpeed = (Path.BoostRemaining > 0.f)
							? Path.BaseSpeed * Path.BoostMultiplier
							: Path.BaseSpeed;

	Path.CurrentSpeed = FMath::FInterpTo(
		Path.CurrentSpeed,
		TargetSpeed,
		DeltaTime,
		5.f   // TODO: Interp speed (tune this)
	);

	if (Path.BoostRemaining > 0) Path.BoostRemaining = FMath::Max(Path.BoostRemaining - DeltaTime, 0);

	Path.CurrentSplineTime += DeltaTime * Path.CurrentSpeed;
	Path.CurrentSplineTime = FMath::Fmod(Path.CurrentSplineTime, Spline->Duration);

	Target->SetWorldLocation(
		Spline->GetLocationAtTime(Path.CurrentSplineTime, ESplineCoordinateSpace::World)
	);
}
