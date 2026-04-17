// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GPUFlock.h"

#include "RenderGraphUtils.h"
#include "GPU/GPUDrawComponent.h"
#include "GPU/Helpers/RDGHelper.h"
#include "GPU/Shaders/ExecuteDeclarations.h"

#include "GPU/Helpers/BitonicSorter.h"

// Sets default values
AGPUFlock::AGPUFlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Target = CreateDefaultSubobject<UStaticMeshComponent>("Target");
	DrawComponent = CreateDefaultSubobject<UGPUDrawComponent>("DrawComponent");
	DrawComponent->GetNiagara()->SetupAttachment(Target);
}

// Called when the game starts or when spawned
void AGPUFlock::BeginPlay()
{
	Super::BeginPlay();
	
	GameInit();
	RenderInit();
}

// Called every frame
void AGPUFlock::Tick(float DeltaTime)
{
	GameTick(DeltaTime);
	RenderTick();
}

void AGPUFlock::GameInit()
{
	DrawComponent->InitGameThread(Settings.BoidCount, PositionBuffer, VelocityBuffer);
}

void AGPUFlock::RenderInit()
{
	ENQUEUE_RENDER_COMMAND(InitPass)([this](FRHICommandListImmediate& RHICmdList)
	{
		BoidsRDGData.ClearPasses();
		BoidsRDGData.InitFence = RHICreateGPUFence(TEXT("BoidsInitFence"));
		FRDGBuilder GraphBuilder(RHICmdList);

		// Construct boids (positions and directions) in GPU
		InitializeBuffers(GraphBuilder);
		// INITIALIZE EVERYTHING!
			
		GraphBuilder.Execute();
		RHICmdList.WriteGPUFence(BoidsRDGData.InitFence);
	});
}

void AGPUFlock::GameTick(float DeltaTime)
{
	LastDeltaTime = DeltaTime;
		
	DrawComponent->UpdateNiagaraBuffer("BoidRenderData", Settings.BoidCount, PositionBuffer.ReadPooled, VelocityBuffer.ReadPooled);
}

void AGPUFlock::RenderTick()
{
	ENQUEUE_RENDER_COMMAND(ComputePass)([this](FRHICommandListImmediate& RHICmdList)
	{
		if (BoidsRDGData.WaitingOnFence()) return;
				
		FRDGBuilder GraphBuilder(RHICmdList);

		// Register buffers before adding passes.
		HashBuffer.RegisterWrite(GraphBuilder, "HashBuffer");
		BoidIndexBuffer.RegisterWrite(GraphBuilder, "BoidIndexBuffer");
		CellOffsetBuffer.RegisterWrite(GraphBuilder, "CellOffsetBuffer");
		
		VelocityBuffer.RegisterReadWrite(GraphBuilder, "InVelocity", "OutVelocity");
		PositionBuffer.RegisterReadWrite(GraphBuilder, "InPosition", "OutPosition");

		// 1. Recreate Hashes
		RebuildHashes(GraphBuilder);
		// 2. Sort Boids by hash
		BitonicSorter::BitonicSort(Settings.BoidCount, Settings.BoidCount, HashBuffer.WriteScopedUAV, BoidIndexBuffer.WriteScopedUAV, BoidsRDGData, GraphBuilder);

		// 3. Rebuild cell offset buffer
		RebuildOffsetBuffer(GraphBuilder);
		// 4. Execute main CS (heading) e.g. calculate velocity
		UpdateBoidHeading(GraphBuilder);
		
		GraphBuilder.Execute();

		PositionBuffer.SwapBuffers();	
		VelocityBuffer.SwapBuffers();
	});
}

void AGPUFlock::Dispose()
{
	ENQUEUE_RENDER_COMMAND(DisposePass)([this](FRHICommandListImmediate& RHICmdList)
	{
		BoidsRDGData.DisposeFence();
	});
}

#pragma region Compute methods
void AGPUFlock::InitializeBuffers(FRDGBuilder& GraphBuilder)
{
	check(IsInRenderingThread())

	TArray<uint32> IntData;
	IntData.SetNum(Settings.BoidCount);

	TArray<FVector3f> Velocities;
	Velocities.SetNum(Settings.BoidCount);
	TArray<FVector3f> Positions;
	Positions.SetNum(Settings.BoidCount);

	const FVector SpawnLocation = GetActorLocation();
	for (auto i = 0; i < Settings.BoidCount; ++i)
	{
		FVector Location, Velocity;
		Location = FMath::VRand() * FMath::RandRange(0.f, 500.f);
		Location += SpawnLocation;
		Velocity = FMath::VRand() * FMath::FRandRange(-Settings.Movement.BaseSpeed, Settings.Movement.BaseSpeed);
		Positions[i] = FVector3f(Location);
		Velocities[i] = FVector3f(Velocity);
	}

	// Position buffer
	FRDGBufferRef PositionReadBufferRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("InPositionBuffer"), sizeof(FVector3f), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(PositionReadBufferRef, Positions.GetData(), Positions.GetTypeSize() * Settings.BoidCount);

	FRDGBufferRef PositionWriteBufferRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("OutPositionBuffer"), sizeof(FVector3f), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(PositionWriteBufferRef, Positions.GetData(), Positions.GetTypeSize() * Settings.BoidCount);
	
	// VelocityBuffer
	FRDGBufferRef VelocityReadBufferRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("InVelocityBuffer"), sizeof(FVector3f), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(VelocityReadBufferRef, Velocities.GetData(), Velocities.GetTypeSize() * Settings.BoidCount);

	FRDGBufferRef VelocityWriteBufferRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("OutVelocityBuffer"), sizeof(FVector3f), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(VelocityWriteBufferRef, Velocities.GetData(), Velocities.GetTypeSize() * Settings.BoidCount);

	// BoidIndexBuffer
	FRDGBufferRef BoidIndexWriteRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("BoidIndexBuffer"), sizeof(uint32), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(BoidIndexWriteRef, IntData.GetData(), IntData.GetTypeSize() * Settings.BoidCount);

	// HashBuffer
	FRDGBufferRef HashBufferRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("HashBuffer"), sizeof(uint32), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(HashBufferRef, IntData.GetData(), IntData.GetTypeSize() * Settings.BoidCount);

	// CellOffsetBuffer
	FRDGBufferRef CellOffsetBufferRef = RDGHelper::CreateStructuredBuffer(GraphBuilder, TEXT("CellOffsetBufferRef"), sizeof(uint32), Settings.BoidCount);
	GraphBuilder.QueueBufferUpload(CellOffsetBufferRef, IntData.GetData(), IntData.GetTypeSize() * Settings.BoidCount);
	// ----
	
	PositionBuffer.ReadPooled = GraphBuilder.ConvertToExternalBuffer(PositionReadBufferRef);
	PositionBuffer.WritePooled = GraphBuilder.ConvertToExternalBuffer(PositionWriteBufferRef);

	VelocityBuffer.ReadPooled = GraphBuilder.ConvertToExternalBuffer(VelocityReadBufferRef);
	VelocityBuffer.WritePooled = GraphBuilder.ConvertToExternalBuffer(VelocityWriteBufferRef);

	BoidIndexBuffer.WritePooled = GraphBuilder.ConvertToExternalBuffer((BoidIndexWriteRef));
	HashBuffer.WritePooled = GraphBuilder.ConvertToExternalBuffer(HashBufferRef);
	CellOffsetBuffer.WritePooled = GraphBuilder.ConvertToExternalBuffer(CellOffsetBufferRef);
	
	PositionBuffer.RegisterReadWrite(GraphBuilder, "InPosition","OutPosition");
	VelocityBuffer.RegisterReadWrite(GraphBuilder, "InVelocity","OutVelocity");
	
	GraphBuilder.QueueBufferExtraction(PositionBuffer.WriteScopedRef, &PositionBuffer.ReadPooled);
	GraphBuilder.QueueBufferExtraction(VelocityBuffer.WriteScopedRef, &VelocityBuffer.ReadPooled);
}

void AGPUFlock::RebuildHashes(FRDGBuilder& GraphBuilder)
{
	TShaderMapRef<FCreateHashesCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FCreateHashesCS::FParameters* PassParams = GraphBuilder.AllocParameters<FCreateHashesCS::FParameters>();
	
	// Params
	PassParams->BoidCount = Settings.BoidCount;
	PassParams->CellSize = Settings.Hash.CellSize;
	
	PassParams->HashBuffer = HashBuffer.WriteScopedUAV;
	PassParams->InPositions = PositionBuffer.ReadScopedSRV;
	PassParams->BoidIndexBuffer = BoidIndexBuffer.WriteScopedUAV;
	
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CreateHashes"), ERDGPassFlags::Compute, ComputeShader, PassParams, ThreadGroupSize(Settings.BoidCount));
}

void AGPUFlock::RebuildOffsetBuffer(FRDGBuilder& GraphBuilder)
{
	TShaderMapRef<FCreateOffsetListCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FCreateOffsetListCS::FParameters* PassParams = GraphBuilder.AllocParameters<FCreateOffsetListCS::FParameters>();

	PassParams->BoidCount = Settings.BoidCount;
	
	PassParams->HashBuffer = HashBuffer.WriteScopedUAV;
	PassParams->BoidIndexBuffer = BoidIndexBuffer.WriteScopedUAV;
	PassParams->CellOffsetBuffer = CellOffsetBuffer.WriteScopedUAV;
	
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CreateHashes"),	ERDGPassFlags::Compute, ComputeShader, PassParams, ThreadGroupSize(Settings.BoidCount));
}

void AGPUFlock::UpdateBoidHeading(FRDGBuilder& GraphBuilder)
{
	TShaderMapRef<FCalculateHeadingCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FCalculateHeadingCS::FParameters* PassParams = GraphBuilder.AllocParameters<FCalculateHeadingCS::FParameters>();
	PassParams->BoidCount = Settings.BoidCount;
	PassParams->CellSize = Settings.Hash.CellSize;
	PassParams->BoidSpeed = Settings.Movement.BaseSpeed;
	PassParams->DeltaTime = LastDeltaTime;

	PassParams->MaxDistanceFromTarget = Settings.Movement.MaxDistanceFromTarget;
	PassParams->MinDistanceFromTarget = Settings.Movement.MinDistanceFromTarget;
	PassParams->SearchModifier = Settings.Hash.SearchModifier;
	PassParams->SeparationDistance = Settings.Movement.SeparationDistance;
	
	PassParams->SeparationStrength = Settings.Strengths.Separation;
	PassParams->AlignmentStrength = Settings.Strengths.Alignment;
	PassParams->CohesionStrength = Settings.Strengths.Cohesion;
	PassParams->TargetStrength = Settings.Strengths.Target;
	PassParams->PlayerAvoidanceStrength = Settings.Strengths.PlayerAvoidance;

	PassParams->Target = FVector3f(Target->GetComponentLocation());
	PassParams->AvoidanceDistance = Settings.Movement.AvoidanceDistance;
	
	PassParams->HashBuffer = HashBuffer.WriteScopedUAV;
	PassParams->CellOffsetBuffer = CellOffsetBuffer.WriteScopedUAV;
	PassParams->BoidIndexBuffer = BoidIndexBuffer.WriteScopedUAV;

	PassParams->InPositions = PositionBuffer.ReadScopedSRV;
	PassParams->OutPositions = PositionBuffer.WriteScopedUAV;
	
	PassParams->InVelocities = VelocityBuffer.ReadScopedSRV;
	PassParams->OutVelocities = VelocityBuffer.WriteScopedUAV;
	
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CalculateHeading"), ERDGPassFlags::Compute, ComputeShader, PassParams, ThreadGroupSize(Settings.BoidCount));
}
#pragma endregion