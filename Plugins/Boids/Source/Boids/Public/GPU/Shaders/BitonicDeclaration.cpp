#include "BitonicDeclaration.h"

bool FBitonicSort_SortCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
}
IMPLEMENT_GLOBAL_SHADER(FBitonicSort_SortCS, "/ComputeShaders/Sort/BitonicSort.usf", "BitonicSort_sort", SF_Compute);

bool FBitonicSort_SortStepCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
}
IMPLEMENT_GLOBAL_SHADER(FBitonicSort_SortStepCS, "/ComputeShaders/Sort/BitonicSort_sortStep.usf", "BitonicSort_sortStep", SF_Compute);

bool FBitonicSort_SortInnerCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
}
IMPLEMENT_GLOBAL_SHADER(FBitonicSort_SortInnerCS, "/ComputeShaders/Sort/BitonicSort_sortInner.usf", "BitonicSort_sortInner", SF_Compute);
