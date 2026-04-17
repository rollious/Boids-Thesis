#pragma once
#include "RenderGraphBuilder.h"

struct BOIDS_API FDoubleBuffer
{
public:
	FDoubleBuffer() = default;

	bool IsInitialized() const { return ReadPooled.IsValid() && WritePooled.IsValid(); }
	bool IsRDGRegistered() const { return ReadScopedRef != nullptr && WriteScopedRef != nullptr; };

	void SwapBuffers()
	{
		ReadPooled.Swap(WritePooled);
	}

	void RegisterReadWrite(FRDGBuilder& GraphBuilder, const FString& ReadName, FString WriteName)
	{
		RegisterSRV(GraphBuilder, ReadPooled, ReadName, ReadScopedRef, ReadScopedSRV);
		RegisterUAV(GraphBuilder, WritePooled, WriteName, WriteScopedRef, WriteScopedUAV);
	}

	~FDoubleBuffer() { ReleaseData(); }

	void ReleaseData()
	{
		ReadPooled.SafeRelease();
		WritePooled.SafeRelease();
		ReadScopedRef = nullptr;
		WriteScopedRef = nullptr;
		ReadScopedSRV = nullptr;
		WriteScopedUAV = nullptr;
	}
private:
	static void RegisterSRV(FRDGBuilder& GraphBuilder, const TRefCountPtr<FRDGPooledBuffer>& Buffer, const FString& Name,
		FRDGBufferRef& OutRDGRef, FRDGBufferSRVRef& OutSRVRef, const ERDGBufferFlags Flags = ERDGBufferFlags::None)
	{
		OutRDGRef = GraphBuilder.RegisterExternalBuffer(Buffer, *Name, Flags);
		OutSRVRef = GraphBuilder.CreateSRV(OutRDGRef);
	}

	static void RegisterUAV(FRDGBuilder& GraphBuilder, const TRefCountPtr<FRDGPooledBuffer>& Buffer, const FString& Name,
		FRDGBufferRef& OutRDGRef, FRDGBufferUAVRef& OutUAVRef, const ERDGBufferFlags Flags = ERDGBufferFlags::None)
	{
		OutRDGRef = GraphBuilder.RegisterExternalBuffer(Buffer, *Name, Flags);
		OutUAVRef = GraphBuilder.CreateUAV(OutRDGRef);
	}
	
public:
	TRefCountPtr<FRDGPooledBuffer> ReadPooled = nullptr;
	TRefCountPtr<FRDGPooledBuffer> WritePooled = nullptr;
	
	FRDGBufferRef ReadScopedRef = nullptr;
	FRDGBufferRef WriteScopedRef = nullptr;
	
	FRDGBufferSRVRef ReadScopedSRV = nullptr;
	FRDGBufferUAVRef WriteScopedUAV = nullptr;
};

struct BOIDS_API FUAVBuffer
{
	FUAVBuffer() = default;
	
	bool IsInitialized() const { return WritePooled.IsValid(); }
	bool IsRDGRegistered() const { return WriteScopedRef != nullptr; };
	
	void RegisterWrite(FRDGBuilder& GraphBuilder, const FString& WriteName)
	{
		RegisterUAV(GraphBuilder, WritePooled, WriteName, WriteScopedRef, WriteScopedUAV);
	}
	
	~FUAVBuffer() { ReleaseData(); }
	void ReleaseData()
	{
		WritePooled.SafeRelease();
		WriteScopedRef = nullptr;
		WriteScopedUAV = nullptr;
	}
private:
	static void RegisterUAV(FRDGBuilder& GraphBuilder, const TRefCountPtr<FRDGPooledBuffer>& Buffer, const FString& Name,
		FRDGBufferRef& OutRDGRef, FRDGBufferUAVRef& OutUAVRef, const ERDGBufferFlags Flags = ERDGBufferFlags::None)
	{
		OutRDGRef = GraphBuilder.RegisterExternalBuffer(Buffer, *Name, Flags);
		OutUAVRef = GraphBuilder.CreateUAV(OutRDGRef);
	}

public:
	FRDGBufferUAVRef WriteScopedUAV = nullptr;
	TRefCountPtr<FRDGPooledBuffer> WritePooled = nullptr;
	FRDGBufferRef WriteScopedRef = nullptr;
};

struct BOIDS_API FSRVBuffer
{
	FSRVBuffer() = default;
	
	bool IsInitialized() const { return ReadPooled.IsValid(); }
	bool IsRDGRegistered() const { return ReadScopedRef != nullptr; };
	
	void RegisterRead(FRDGBuilder& GraphBuilder, const FString& ReadName)
	{
		RegisterSRV(GraphBuilder, ReadPooled, ReadName, ReadScopedRef, ReadScopedSRV);
	}
	
	~FSRVBuffer() { ReleaseData(); }
	void ReleaseData()
	{
		ReadPooled.SafeRelease();
		ReadScopedRef = nullptr;
		ReadScopedSRV = nullptr;
	}
private:
	static void RegisterSRV(FRDGBuilder& GraphBuilder, const TRefCountPtr<FRDGPooledBuffer>& Buffer, const FString& Name,
		FRDGBufferRef& OutRDGRef, FRDGBufferSRVRef& OutSRVRef, const ERDGBufferFlags Flags = ERDGBufferFlags::None)
	{
		OutRDGRef = GraphBuilder.RegisterExternalBuffer(Buffer, *Name, Flags);
		OutSRVRef = GraphBuilder.CreateSRV(OutRDGRef);
	}

public:
	FRDGBufferSRVRef ReadScopedSRV = nullptr;
	TRefCountPtr<FRDGPooledBuffer> ReadPooled = nullptr;
	FRDGBufferRef ReadScopedRef = nullptr;
};

struct FBoidsRDGData
{
public:
	FBoidsRDGData()
	{
		InitPass.SetNum(0);
		ExecutePass.SetNum(0);
		InitFence = nullptr;
	}

	FBoidsRDGData(const int32 InitPassSize, const int32 ExecutePassSize)
	{
		InitPass.SetNum(InitPassSize);
		ExecutePass.SetNum(ExecutePassSize);
		InitFence = nullptr;
	}

	void ClearPasses()
	{
		for (FRDGPassRef& Pass : InitPass) Pass = nullptr; // simple for loop instead?
		for (FRDGPassRef& Pass : ExecutePass) Pass = nullptr;
	}

	void DisposeFence()
	{
		if (!InitFence.IsValid()) return;
		InitFence->Clear();
		InitFence = nullptr;
	}

	void CheckPass()
	{
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Green, FString::Printf(TEXT("%d"), TruePassIndex));
		TruePassIndex = 0;
	}

	void AddExecutePass(const FRDGPassRef Pass)
	{
		ExecutePass[PassIndex] = Pass;
		const int NextIndex = PassIndex + 1;
		PassIndex = NextIndex < ExecutePass.Num() ? NextIndex : 0;
		TruePassIndex++;
	}

	bool WaitingOnFence() const { return (!InitFence.IsValid() || !InitFence->Poll()); }
	
public:
	TArray<FRDGPassRef> InitPass;
	TArray<FRDGPassRef> ExecutePass;
	FGPUFenceRHIRef InitFence;
private:
	int PassIndex = 0;
	int TruePassIndex = 0; // for debug checking 
};