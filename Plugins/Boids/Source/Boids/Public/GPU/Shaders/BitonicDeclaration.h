#pragma once
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"

class FBitonicSort_SortCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FBitonicSort_SortCS)
	SHADER_USE_PARAMETER_STRUCT(FBitonicSort_SortCS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, job_params)
		SHADER_PARAMETER(uint32, itemCount)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, comparisonBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, indexBuffer)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FBitonicSort_SortStepCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FBitonicSort_SortStepCS);
	SHADER_USE_PARAMETER_STRUCT(FBitonicSort_SortStepCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, job_params)
		SHADER_PARAMETER(uint32, itemCount)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, comparisonBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, indexBuffer)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FBitonicSort_SortInnerCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FBitonicSort_SortInnerCS);
	SHADER_USE_PARAMETER_STRUCT(FBitonicSort_SortInnerCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, job_params)
		SHADER_PARAMETER(uint32, itemCount)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint>, comparisonBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint>, indexBuffer)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};
