// Fill out your copyright notice in the Description page of Project Settings.


#include "GPU/GPUDrawComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BoidNiagaraDataInterface.h"

UGPUDrawComponent::UGPUDrawComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->bAutoActivate = true;

	//if (GetOwner()) Niagara->SetupAttachment(GetOwner()->)
}

void UGPUDrawComponent::InitGameThread(const int32 BoidCount, FDoubleBuffer& PositionBuffer, FDoubleBuffer& VelocityBuffer)
{
	check(IsInGameThread());

	Niagara->SetAsset(NiagaraSystem.LoadSynchronous());
	// Update niagara
	if (IsValid(Niagara) && IsValid(Niagara->GetAsset()))
	{
		// CONSTANT PARAMETERS
		//UpdateNiagaraBuffer("BoidRenderData", BoidCount, nullptr, nullptr);
		
		Niagara->SetIntParameter("BoidCount", BoidCount);
		
		// DYNAMIC PARAMETERS
		UpdateNiagaraBuffer("BoidRenderData", BoidCount, PositionBuffer.ReadPooled, VelocityBuffer.ReadPooled);
		// mesh scale & world scale
		
		Niagara->Activate(true);
	}
}

void UGPUDrawComponent::UpdateNiagaraBuffer(const FName OverrideName, const uint32 BoidCount,
                                            const TRefCountPtr<FRDGPooledBuffer>& PositionBuffer, const TRefCountPtr<FRDGPooledBuffer>& DirectionBuffer) const
{
	if (UBoidNiagaraDataInterface* Interface = UNiagaraFunctionLibrary::GetDataInterface<UBoidNiagaraDataInterface>(Niagara, OverrideName))
		Interface->SetData(BoidCount, PositionBuffer, DirectionBuffer);
}

void UGPUDrawComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Niagara->Activate(true);
}
