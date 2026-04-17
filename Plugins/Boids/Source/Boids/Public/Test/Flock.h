// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/BoidConstants.h"
#include "GameFramework/Actor.h"

#include "Flock.generated.h"

class ABoidActor;
class USpatialHash;

UCLASS(Abstract)
class BOIDS_API UFlock : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UFlock();

	virtual void InitFlock(uint32 Amount, const FTestSettings& Settings);
	virtual void Work(float DeltaSeconds, const FTestSettings& Settings);

protected:
	FBoidsSOA BoidSOA;
	
	TArray<TObjectPtr<ABoidActor>> BoidActors;
	
	UPROPERTY()
	TObjectPtr<USpatialHash> Grid;
};
