#pragma once
#include "NiagaraDataInterface.h"
#include "BoidNiagaraDataInterface.generated.h"

#pragma region Structs
struct FBoidsBufferAttribute
{
	static TConstArrayView<FBoidsBufferAttribute> GetBoidBufferAttributes()
	{
		static const TArray<FBoidsBufferAttribute> Attributes =
		{
			FBoidsBufferAttribute(TEXT("BoidPosition"), FNiagaraTypeDefinition::GetVec3Def(), FText::GetEmpty()),
			FBoidsBufferAttribute(TEXT("BoidVelocity"), FNiagaraTypeDefinition::GetVec3Def(), FText::GetEmpty())
		};

		return MakeArrayView(Attributes);
	}
	
	FBoidsBufferAttribute(const TCHAR* InName, const FNiagaraTypeDefinition& TypeDef, const FText& Desc)
		: AttributeName(InName), TypeDef(TypeDef), Description(Desc)
	{
		FString TempName = TEXT("Get");
		TempName += AttributeName;
		DisplayFunctionName = FName(TempName);
	}

	const TCHAR* AttributeName;
	FName DisplayFunctionName; // Get{BoidPosition}
	FNiagaraTypeDefinition TypeDef;
	FText Description;
};
#pragma endregion

UCLASS(EditInlineNew, Category ="Boids", meta = (DisplayName = "BoidRenderData"))
class BOIDS_API UBoidNiagaraDataInterface : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FBoidParameters, )
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, InPositions)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, InVelocities)
		SHADER_PARAMETER(uint32, BoidCount)
	END_SHADER_PARAMETER_STRUCT()
public:
	void SetData(const uint32 Count, const TRefCountPtr<FRDGPooledBuffer>& PooledPos, const TRefCountPtr<FRDGPooledBuffer>& PooledVel)
	{
		BoidCount = Count;
		PooledPositions = PooledPos;
		PooledVelocities = PooledVel;
	}

public:
#pragma region Interface virtuals
	virtual void PostInitProperties() override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return Target == ENiagaraSimTarget::GPUComputeSim; }
	
	virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	virtual bool HasPreSimulateTick() const override { return true; }
	virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize() const override;

	// void ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance) override;
#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
	virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
#endif
#pragma endregion
private:
	TRefCountPtr<FRDGPooledBuffer> PooledPositions = nullptr;
	TRefCountPtr<FRDGPooledBuffer> PooledVelocities = nullptr;
	int32 BoidCount = 0;
};

