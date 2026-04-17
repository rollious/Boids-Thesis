// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidActor.generated.h"

/**
 * A single-threaded boid
 */
UCLASS()
class BOIDS_API ABoidActor : public AActor
{
	GENERATED_BODY()

public:
	ABoidActor();
	
	UFUNCTION(BlueprintCallable)
	void DrawBounds() const
	{
		DrawDebugBox(GetWorld(), RootComponent->Bounds.Origin, RootComponent->Bounds.BoxExtent, FColor::Red, false, -1, 0, 2);
	}
};
