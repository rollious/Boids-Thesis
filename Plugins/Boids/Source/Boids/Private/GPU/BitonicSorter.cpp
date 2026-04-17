#include "GPU/Helpers/BitonicSorter.h"
#include "GPU/Shaders/BitonicDeclaration.h"
#include "RenderGraphUtils.h"
#include "Boids/Public/GPU/Structs/GPUStructs.h"

void BitonicSorter::BitonicSort(uint32 MaxCount, uint32 NumItems,
                                FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort, FBoidsRDGData& RDGData, FRDGBuilder& GraphBuilder)
{
	int ThreadCount = ((NumItems - 1) >> 9) + 1;
	bool Done = true;

	Sort(MaxCount, NumItems, ComparisonBuffer, BufferToSort, GraphBuilder, ThreadCount, Done);
	
	int Presorted = 512;
	while (!Done)
	{
		// Incremental sorting:
		Done = true;

		// Prepare thread group description data:
		uint32 NumThreadGroups = 0;
		if (MaxCount > static_cast<uint32>(Presorted))
		{
			if (MaxCount > static_cast<uint32>(Presorted) * 2) Done = false;
			
			uint32 Pow2 = Presorted;
			while (Pow2 < MaxCount) Pow2 *= 2;
			NumThreadGroups = Pow2 >> 9;
		}
		
		FIntVector JobParams;
		// StepSort
		uint32 MergeSize = Presorted * 2;
		for (uint32 MergeSubSize = MergeSize >> 1; MergeSubSize > 256; MergeSubSize = MergeSubSize >> 1)
				SortStep(NumItems, ComparisonBuffer, BufferToSort, GraphBuilder, NumThreadGroups, JobParams, MergeSize, MergeSubSize);
		
		SortInner(NumItems, ComparisonBuffer, BufferToSort, GraphBuilder, NumThreadGroups, JobParams);
		Presorted *= 2;
	}
}

void BitonicSorter::Sort(uint32 MaxCount, uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort, FRDGBuilder& GraphBuilder, int ThreadCount, bool& Done)
{
	TShaderMapRef<FBitonicSort_SortCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FBitonicSort_SortCS::FParameters* PassParams = GraphBuilder.AllocParameters<FBitonicSort_SortCS::FParameters>();
	PassParams->itemCount = NumItems;
	PassParams->comparisonBuffer = ComparisonBuffer;
	PassParams->indexBuffer = BufferToSort;

	const unsigned int NumThreadGroups = ((MaxCount - 1) >> 9) + 1;
	if (NumThreadGroups > 1) Done = false;

	FComputeShaderUtils::AddPass(
		GraphBuilder, RDG_EVENT_NAME("BitonicSort_Sort"),
		ERDGPassFlags::Compute | ERDGPassFlags::NeverCull,
		ComputeShader, PassParams, FIntVector(ThreadCount, 1, 1));
}

void BitonicSorter::SortStep(uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort, FRDGBuilder& GraphBuilder, uint32 NumThreadGroups, FIntVector& JobParams, uint32 MergeSize, uint32 MergeSubSize)
{
	JobParams.X = MergeSubSize;
	if (MergeSubSize == MergeSize >> 1)
	{
		JobParams.Y = (2 * MergeSubSize - 1);
		JobParams.Z = -1;
	}
	else
	{
		JobParams.Y = MergeSubSize;
		JobParams.Z = 1;
	}

	TShaderMapRef<FBitonicSort_SortStepCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FBitonicSort_SortStepCS::FParameters* PassParams = GraphBuilder.AllocParameters<FBitonicSort_SortStepCS::FParameters>();
	PassParams->itemCount = NumItems;
	PassParams->job_params = JobParams;
	PassParams->comparisonBuffer = ComparisonBuffer;
	PassParams->indexBuffer = BufferToSort;

	FComputeShaderUtils::AddPass(
		GraphBuilder, RDG_EVENT_NAME("BitonicSort_SortStep"),
		ERDGPassFlags::Compute | ERDGPassFlags::NeverCull,
		ComputeShader, PassParams, FIntVector(NumThreadGroups, 1, 1));
}

void BitonicSorter::SortInner(uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort, FRDGBuilder& GraphBuilder, uint32 NumThreadGroups, FIntVector JobParams)
{
	TShaderMapRef<FBitonicSort_SortInnerCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FBitonicSort_SortInnerCS::FParameters* PassParams = GraphBuilder.AllocParameters<FBitonicSort_SortInnerCS::FParameters>();
	PassParams->job_params = JobParams;
	PassParams->itemCount = NumItems;
	PassParams->comparisonBuffer = ComparisonBuffer;
	PassParams->indexBuffer = BufferToSort;
	
	FComputeShaderUtils::AddPass(
		GraphBuilder, RDG_EVENT_NAME("BitonicSort_SortInner"),
		ERDGPassFlags::Compute | ERDGPassFlags::NeverCull,
		ComputeShader, PassParams, FIntVector(NumThreadGroups, 1, 1));
}