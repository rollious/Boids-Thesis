#pragma once
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderGraphResources.h"


class BOIDS_API RDGHelper
{
public:
	static FRDGBufferRef CreateStructuredBuffer(FRDGBuilder& GraphBuilder, const TCHAR* Name,
		const uint32 BytesPerElement, const uint32 NumElements, const bool bReadOnly = false)
	{
		FRDGBufferDesc Desc = FRDGBufferDesc::CreateStructuredDesc(BytesPerElement, NumElements);
		if (bReadOnly) EnumRemoveFlags(Desc.Usage, EBufferUsageFlags::UnorderedAccess);
		return GraphBuilder.CreateBuffer(Desc, Name);
	}

	static FRDGBufferUAVRef CreateUAVBuffer(FRDGBuilder& GraphBuilder, const TCHAR* Name,
		const uint32 BytesPerElement, const uint32 NumElements)
	{
		FRDGBufferRef Buffer = CreateStructuredBuffer(GraphBuilder, Name, BytesPerElement, NumElements);
		return GraphBuilder.CreateUAV(Buffer);
	}

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

	BEGIN_SHADER_PARAMETER_STRUCT(FCopyBufferParams, )
		RDG_BUFFER_ACCESS(SrcBuffer, ERHIAccess::CopySrc)
		RDG_BUFFER_ACCESS(DstBuffer, ERHIAccess::CopySrc)
	END_SHADER_PARAMETER_STRUCT()
	
	static FRDGPassRef AddCopyBufferPass(FRDGBuilder& GraphBuilder, FRDGBufferRef DstBuffer, uint64 DstOffset,
		FRDGBufferRef SrcBuffer, uint64 SrcOffset, uint64 NumBytes)
	{
		check(SrcBuffer);
		check(DstBuffer);

		FCopyBufferParams* Params = GraphBuilder.AllocParameters<FCopyBufferParams>();
		Params->SrcBuffer= SrcBuffer;
		Params->DstBuffer= DstBuffer;
		FRDGPassRef CopyPass = GraphBuilder.AddPass(
			RDG_EVENT_NAME("CopyBuffer(%s Size=%ubytes", SrcBuffer->Name, SrcBuffer->Desc.GetSize()),
			Params, ERDGPassFlags::Copy,
			[&Params, SrcBuffer, DstBuffer, SrcOffset, DstOffset, NumBytes](FRHICommandList& RHICmdList)
			{
				RHICmdList.CopyBufferRegion(DstBuffer->GetRHI(), DstOffset, SrcBuffer->GetRHI(), SrcOffset, NumBytes);
			});
		return CopyPass;
	}
	
	static FRDGPassRef AddCopyBufferPass(FRDGBuilder& GraphBuilder, const FRDGBufferRef DstBuffer, const FRDGBufferRef SrcBuffer)
	{
		check(SrcBuffer);
		check(DstBuffer);
		const uint64 NumBytes = SrcBuffer->Desc.NumElements * SrcBuffer->Desc.BytesPerElement;
		return AddCopyBufferPass(GraphBuilder, DstBuffer, 0, SrcBuffer, 0, NumBytes);
	}
};