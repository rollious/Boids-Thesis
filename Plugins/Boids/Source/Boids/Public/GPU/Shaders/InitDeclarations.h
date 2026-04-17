#pragma once

#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"

class FInitializeBuffersCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FInitializeBuffersCS);
	SHADER_USE_PARAMETER_STRUCT(FInitializeBuffersCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float3>, OutPositions)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float3>, OutVelocities)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint>, BoidIndexBuffer)
		SHADER_PARAMETER(uint32, BoidCount)
		SHADER_PARAMETER(float, BoidSpeed)
		SHADER_PARAMETER(uint32, RandSeed)

		SHADER_PARAMETER(FMatrix44f, BoundsMatrix)
		SHADER_PARAMETER(FMatrix44f, BoundsInverseMatrix)
		SHADER_PARAMETER(float, BoundsRadius)
	END_SHADER_PARAMETER_STRUCT()
	
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
};
IMPLEMENT_GLOBAL_SHADER(FInitializeBuffersCS, "/ComputeShaders/Boid.usf", "InitializeBuffers", SF_Compute);