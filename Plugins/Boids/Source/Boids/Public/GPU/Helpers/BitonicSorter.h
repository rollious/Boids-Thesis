#pragma once

struct FBoidsRDGData;

class BitonicSorter
{
public:
	static void BitonicSort(uint32 MaxCount, uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort,
	                        FRDGBuilder& GraphBuilder);
private:
	static void Sort(uint32 MaxCount, uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort,
					 FRDGBuilder& GraphBuilder, int ThreadCount, bool& Done);
	static void SortStep(uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort,
						 FRDGBuilder& GraphBuilder, uint32 NumThreadGroups, FIntVector& JobParams, uint32 MergeSize,
						 uint32 MergeSubSize);
	static void SortInner(uint32 NumItems, FRDGBufferUAVRef ComparisonBuffer, FRDGBufferUAVRef BufferToSort,
						  FRDGBuilder& GraphBuilder, uint32 NumThreadGroups, FIntVector JobParams);
};
