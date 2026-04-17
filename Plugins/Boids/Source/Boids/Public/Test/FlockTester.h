// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingleFlock.h"
#include "Core/BoidConstants.h"
#include "GameFramework/Actor.h"
#include "FlockTester.generated.h"

class UFlock;
class USingleFlock;
class UGPUFlock;
class UMultiFlock;

class ABoidActor;

UCLASS()
class BOIDS_API AFlockTester : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlockTester();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category = "00. MANAGER|SETTINGS")
	EFlockType FlockType = EFlockType::SINGLE;

	UPROPERTY(EditAnywhere, Category = "00. MANAGER")
	FTestSettings TestSettings;
	
	UPROPERTY()
	TObjectPtr<USingleFlock> SingleFlock;
	
	UPROPERTY()
	TObjectPtr<UMultiFlock> MultiFlock;
	
private:
	void SelectFlock();
	
	FBoidsSOA BoidSOA;
	TArray<TObjectPtr<ABoidActor>> BoidActors;

	UPROPERTY()
	TObjectPtr<UFlock> ActiveFlock = nullptr;
};
