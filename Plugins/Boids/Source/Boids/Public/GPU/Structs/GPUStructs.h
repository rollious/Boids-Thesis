#pragma once
#include "RenderGraphBuilder.h"

struct BOIDS_API FDoubleBuffer
{
public:
	FDoubleBuffer() = default;

	bool IsInitialized() const { return ReadPooled.IsValid() && WritePooled.IsValid(); }
	bool IsRDGRegistered() const { return ReadScopedRef != nullptr && WriteScopedRef != nullptr; };

	void SwapBuffers() { ReadPooled.Swap(WritePooled); }

	void RegisterReadWrite(FRDGBuilder& GraphBuilder, const FString& ReadName, const FString& WriteName)
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
		InitFence = nullptr;
	}

	explicit FBoidsRDGData(const int32 InitPassSize)
	{
		InitPass.SetNum(InitPassSize);
		InitFence = nullptr;
	}

	void ClearPasses()
	{
		for (FRDGPassRef& Pass : InitPass) Pass = nullptr;
	}

	void DisposeFence()
	{
		if (!InitFence.IsValid()) return;
		InitFence->Clear();
		InitFence = nullptr;
	}

	bool WaitingOnFence() const { return (!InitFence.IsValid() || !InitFence->Poll()); }
	
public:
	TArray<FRDGPassRef> InitPass;
	FGPUFenceRHIRef InitFence;
};