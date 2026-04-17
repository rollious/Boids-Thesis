#include "BoidNiagaraDataInterface.h"
#include "NiagaraShaderParametersBuilder.h"
#include "NiagaraGpuComputeDispatchInterface.h"
#include "NiagaraSystemInstance.h"
#include "GPU/Helpers/RDGHelper.h"

// What needs to be included in the template?
static const TCHAR* TemplateShaderFile = TEXT("/ComputeShaders/Niagara/BoidBufferTemplate.ush");

#pragma region Structs
struct FNDIBoidInstanceData_GT
{
	TRefCountPtr<FRDGPooledBuffer> BoidPositions;
	TRefCountPtr<FRDGPooledBuffer> BoidVelocities;
	int32 BoidCount;

	void ReleaseData()
	{
		BoidCount = 0;
		BoidPositions.SafeRelease();
		BoidVelocities.SafeRelease();
	}

	~FNDIBoidInstanceData_GT() { ReleaseData(); }
};
struct FNDIBoidInstanceData_RT
{
	TRefCountPtr<FRDGPooledBuffer> PooledBoidPositions;
	FRDGBufferRef ScopedPosRef = nullptr;
	FRDGBufferSRVRef ScopedPosSRV = nullptr;
	
	TRefCountPtr<FRDGPooledBuffer> PooledBoidVelocities;
	FRDGBufferRef ScopedVelRef = nullptr;
	FRDGBufferSRVRef ScopedVelSRV = nullptr;
	
	int32 BoidCount;

	void ReleaseData()
	{
		BoidCount = 0;
		PooledBoidPositions.SafeRelease();
		PooledBoidVelocities.SafeRelease();
	}

	void UpdateData(const FNDIBoidInstanceData_GT& InstanceData)
	{
		ReleaseData();
		PooledBoidPositions = InstanceData.BoidPositions;
		PooledBoidVelocities = InstanceData.BoidVelocities;
		BoidCount = InstanceData.BoidCount;
	}

	~FNDIBoidInstanceData_RT() { ReleaseData(); }
};

// This proxy is used to safely copy data between game thread and render thread
struct FBoidProxy : public FNiagaraDataInterfaceProxy
{
	/**
	 * The size of the data this class will provide to ProvidePerInstanceDataForRenderThread.
	 * MUST be 16 byte aligned!
	 */ // Why return 0 then?
	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override { return 0; }

	// Why not consumed?
	virtual void ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID) override
	{
		
	}
	// List of proxy data for each system instance
	TMap<FNiagaraSystemInstanceID, FNDIBoidInstanceData_RT> SystemInstancesToInstanceData;
};
#pragma endregion

UBoidNiagaraDataInterface::UBoidNiagaraDataInterface(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Proxy.Reset(new FBoidProxy());
}

#pragma region Interface virtuals
// this registers our custom DI with Niagara
void UBoidNiagaraDataInterface::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject)) return;
	ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
	FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
}

// This fills in the expected parameter bindings we use to send data to the GPU
void UBoidNiagaraDataInterface::BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FBoidParameters>();
}

// This fills in the parameters to send to the GPU
void UBoidNiagaraDataInterface::SetShaderParameters(
	const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	check(IsInRenderingThread())
	
	FBoidParameters* Params = Context.GetParameterNestedStruct<FBoidParameters>();
	FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();

	FBoidProxy& DataInterfaceProxy = Context.GetProxy<FBoidProxy>();
	if (FNDIBoidInstanceData_RT* RTInstanceData = DataInterfaceProxy.SystemInstancesToInstanceData.Find(Context.GetSystemInstanceID()))
	{
		if (RTInstanceData->PooledBoidPositions && RTInstanceData->PooledBoidVelocities != nullptr && RTInstanceData->BoidCount > 0)
		{
			RDGHelper::RegisterSRV(GraphBuilder, RTInstanceData->PooledBoidPositions,TEXT("NDI_BoidPositions"), RTInstanceData->ScopedPosRef, RTInstanceData->ScopedPosSRV);
			RDGHelper::RegisterSRV(GraphBuilder, RTInstanceData->PooledBoidVelocities,TEXT("NDI_BoidVelocities"), RTInstanceData->ScopedVelRef, RTInstanceData->ScopedVelSRV);
		}

		if (RTInstanceData->ScopedPosRef != nullptr && RTInstanceData->ScopedVelRef != nullptr)
		{
			Params->InPositions = RTInstanceData->ScopedPosSRV;
			Params->InVelocities = RTInstanceData->ScopedVelSRV;
			Params->BoidCount = RTInstanceData->BoidCount;
			return;
		}
	}

	// What the hell is happening here?
	// Obviously initialize buffers, but wth is the pixel format?
	Params->InPositions = Context.GetComputeDispatchInterface().GetEmptyBufferSRV(GraphBuilder, PF_A16B16G16R16);
	Params->InVelocities = Context.GetComputeDispatchInterface().GetEmptyBufferSRV(GraphBuilder, PF_A16B16G16R16);
	Params->BoidCount = 0;
}

// This ticks on the game thread and lets us do work to initialize the instance data.
// If you need to do work on the gathered instance data after the simulation is done, use PerInstanceTickPostSimulate() instead. 
bool UBoidNiagaraDataInterface::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FNDIBoidInstanceData_GT* InstanceData = static_cast<FNDIBoidInstanceData_GT*>(PerInstanceData);
	if (!InstanceData) return true;

	bool A = BoidCount == 0 || !PooledPositions.IsValid() || !PooledVelocities.IsValid();
	if (A) return false;

	bool B = InstanceData->BoidCount > 0 && InstanceData->BoidCount == BoidCount;
	bool C = (InstanceData->BoidPositions.IsValid() && InstanceData->BoidPositions->GetSize() == PooledPositions->GetSize())
		&& InstanceData->BoidVelocities.IsValid() && InstanceData->BoidVelocities->GetSize() == PooledVelocities->GetSize();
	if (B && C) return false;

	// Set instance data from the game thread, so we can work on it in the render thread
	InstanceData->BoidCount = BoidCount;
	InstanceData->BoidPositions = PooledPositions; // InstanceData->BoidPositions = Game thread data.
	InstanceData->BoidVelocities = PooledVelocities;
	ENQUEUE_RENDER_COMMAND(UpdateProxy)(
		[RTProxy = GetProxyAs<FBoidProxy>(), RTInstanceID = SystemInstance->GetId(), InstanceDataGT = InstanceData](FRHICommandListImmediate& RHICmdList)
		{
			FNDIBoidInstanceData_RT* InstanceDataRT = &RTProxy->SystemInstancesToInstanceData.FindOrAdd(RTInstanceID);
			InstanceDataRT->UpdateData(*InstanceDataGT);
		});
	return false;
}

// creates a new data object to store our position in.
// Don't keep transient data on the data interface object itself, only use per instance data!
bool UBoidNiagaraDataInterface::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	check(Proxy);
	FNDIBoidInstanceData_GT* InstanceData = new (PerInstanceData)FNDIBoidInstanceData_GT();
	return true;
}

// clean up RT instances
void UBoidNiagaraDataInterface::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FNDIBoidInstanceData_GT* InstanceData = static_cast<FNDIBoidInstanceData_GT*>(PerInstanceData);
	InstanceData->~FNDIBoidInstanceData_GT();
	
	ENQUEUE_RENDER_COMMAND(RemoveProxy)(
		[RTProxy = GetProxyAs<FBoidProxy>(), InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& RHICmdList)
		{
			RTProxy->SystemInstancesToInstanceData.Remove(InstanceID);
		});
}

int32 UBoidNiagaraDataInterface::PerInstanceDataSize() const
{
	return sizeof(FNDIBoidInstanceData_GT);
}

#if WITH_EDITORONLY_DATA
// this lists all the functions our DI provides
void UBoidNiagaraDataInterface::GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	const TConstArrayView<FBoidsBufferAttribute> Attributes = FBoidsBufferAttribute::GetBoidBufferAttributes();
	OutFunctions.Reserve(OutFunctions.Num() + Attributes.Num());
	for (const auto& Attribute : Attributes)
	{
		FNiagaraFunctionSignature& Sig = OutFunctions.AddDefaulted_GetRef();
		Sig.Name = Attribute.DisplayFunctionName;
		Sig.Description = Attribute.Description;
		Sig.bMemberFunction = true;
		Sig.bRequiresContext = false;
		Sig.bSupportsCPU = false;
		
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("BoidsBufferInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ID")));
		
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsValid")));
		Sig.AddOutput(FNiagaraVariable(Attribute.TypeDef, Attribute.AttributeName));
	}
}

// this lets the niagara compiler know that it needs to recompile an effect when our hlsl file changes
bool UBoidNiagaraDataInterface::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
	if (!Super::AppendCompileHash(InVisitor)) return false;
	InVisitor->UpdateShaderFile(TemplateShaderFile);
	InVisitor->UpdateShaderParameters<FBoidParameters>();
	return true;
}

// This can be used to provide the hlsl code for gpu scripts. If the DI supports only cpu implementations, this is not needed.
// We don't need to actually print our function code to OutHLSL here because we use a template file that gets appended in GetParameterDefinitionHLSL().
// If the hlsl function is so simple that it does not need bound shader parameters, then this method can be used instead of GetParameterDefinitionHLSL.
bool UBoidNiagaraDataInterface::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
	for (const auto& Attribute : FBoidsBufferAttribute::GetBoidBufferAttributes())
		if (FunctionInfo.DefinitionName == Attribute.DisplayFunctionName) return true;

	return false;
}

// this loads our hlsl template script file and {WHAT?!?!?}
void UBoidNiagaraDataInterface::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	FString& OutHLSL)
{
	const TMap<FString, FStringFormatArg> TemplateArgs =
	{
		{TEXT("ParameterName"), ParamInfo.DataInterfaceHLSLSymbol}	
	};
	AppendTemplateHLSL(OutHLSL, TemplateShaderFile, TemplateArgs);
}
#endif
#pragma endregion