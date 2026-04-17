// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/FlockTester.h"

#include "Test/Flock.h"
#include "Test/MultiFlock.h"
#include "Test/SingleFlock.h"


// Sets default values
AFlockTester::AFlockTester()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SingleFlock = CreateDefaultSubobject<USingleFlock>(TEXT("SingleFlock"));
	MultiFlock  = CreateDefaultSubobject<UMultiFlock>(TEXT("MultiFlock"));
}

// Called when the game starts or when spawned
void AFlockTester::BeginPlay()
{
	Super::BeginPlay();
	
	SelectFlock();
	if (ActiveFlock) ActiveFlock->InitFlock(TestSettings.BoidAmount, TestSettings);
}

// Called every frame
void AFlockTester::Tick(float DeltaTime)
{
	if (ActiveFlock) ActiveFlock->Work(DeltaTime, TestSettings);
}

void AFlockTester::SelectFlock()
{
	switch (FlockType)
	{
		case EFlockType::SINGLE: ActiveFlock = SingleFlock; break;
		case EFlockType::MULTI:  ActiveFlock = MultiFlock;  break;
	}
}