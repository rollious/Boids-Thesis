#include "ExecuteDeclarations.h"
#include "DataDrivenShaderPlatformInfo.h"

// "/ComputeShaders/SpatialHash.usf" --> See Boids.cpp

bool FCreateHashesCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
}
IMPLEMENT_GLOBAL_SHADER(FCreateHashesCS, "/ComputeShaders/SpatialHash.usf", "CreateHashes", SF_Compute)

bool FCreateOffsetListCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
}
IMPLEMENT_GLOBAL_SHADER(FCreateOffsetListCS, "/ComputeShaders/SpatialHash.usf", "CreateOffsetList", SF_Compute);

bool FCalculateHeadingCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
}
IMPLEMENT_GLOBAL_SHADER(FCalculateHeadingCS, "/ComputeShaders/Boid.usf", "CalculateHeading", SF_Compute);
