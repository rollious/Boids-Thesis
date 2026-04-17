#pragma once
#include "ShaderParameterStruct.h"

class FCreateHashesCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FCreateHashesCS)
	SHADER_USE_PARAMETER_STRUCT(FCreateHashesCS, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, InPositions)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, BoidIndexBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, HashBuffer)
		
		SHADER_PARAMETER(uint32, BoidCount)

		SHADER_PARAMETER(float, CellSize)
	END_SHADER_PARAMETER_STRUCT()

	// Ensure that the shader can be successfully compiled in the given platform.
	// e.g. "Compile this shader only on platforms that support at least ES3.1"
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCreateOffsetListCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCreateOffsetListCS);
	SHADER_USE_PARAMETER_STRUCT(FCreateOffsetListCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, BoidIndexBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, HashBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, CellOffsetBuffer)
				
		SHADER_PARAMETER(uint32, BoidCount)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};


class FCalculateHeadingCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCalculateHeadingCS)
	SHADER_USE_PARAMETER_STRUCT(FCalculateHeadingCS, FGlobalShader)
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, InPositions) // Located in SpatialHash.usf
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float3>, OutPositions) // Located in SpatialHash.usf
	
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, InVelocities)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float3>, OutVelocities)
		
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, BoidIndexBuffer) // Located in SpatialHash.usf
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, HashBuffer) // Located in SpatialHash.usf
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<uint32>, CellOffsetBuffer) // Located in SpatialHash.usf

		SHADER_PARAMETER(FVector3f, Target)
		
		SHADER_PARAMETER(float, AlignmentStrength)
		SHADER_PARAMETER(float, CohesionStrength)
		SHADER_PARAMETER(float, SeparationStrength)
		SHADER_PARAMETER(float, TargetStrength)
		SHADER_PARAMETER(float, PlayerAvoidanceStrength)

		SHADER_PARAMETER(float, SearchModifier)
		SHADER_PARAMETER(float, SeparationDistance)
		SHADER_PARAMETER(float, AvoidanceDistance)
		SHADER_PARAMETER(float, MaxDistanceFromTarget)
		SHADER_PARAMETER(float, MinDistanceFromTarget)
	
		SHADER_PARAMETER(float, BoidSpeed)
		SHADER_PARAMETER(float, DeltaTime)
		SHADER_PARAMETER(float, CellSize)  // Located in SpatialHash.usf
			
		SHADER_PARAMETER(uint32, BoidCount)  // Located in SpatialHash.usf
		SHADER_PARAMETER(uint32, PlayerCount)  // Located in SpatialHash.usf
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};