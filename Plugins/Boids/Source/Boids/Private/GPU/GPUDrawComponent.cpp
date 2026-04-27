// Fill out your copyright notice in the Description page of Project Settings.


#include "GPU/GPUDrawComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BoidNiagaraDataInterface.h"

UGPUDrawComponent::UGPUDrawComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->bAutoActivate = true; 
}

void UGPUDrawComponent::InitGameThread(const int32 BoidCount, const FDoubleBuffer& PositionBuffer, const FDoubleBuffer& VelocityBuffer) const
{
	check(IsInGameThread());
	Niagara->SetAsset(NiagaraSystem.LoadSynchronous());
	if (IsValid(Niagara) && IsValid(Niagara->GetAsset())) // Update Niagara
	{
		Niagara->SetIntParameter("BoidCount", BoidCount);
		UpdateNiagaraBuffer("BoidRenderData", BoidCount, PositionBuffer.ReadPooled, VelocityBuffer.ReadPooled);
		Niagara->Activate(true);
	}
}


void UGPUDrawComponent::UpdateNiagaraBuffer(const FName OverrideName, const uint32 BoidCount,
                                            const TRefCountPtr<FRDGPooledBuffer>& PositionBuffer, const TRefCountPtr<FRDGPooledBuffer>& DirectionBuffer) const
{
	if (UBoidNiagaraDataInterface* Interface = UNiagaraFunctionLibrary::GetDataInterface<UBoidNiagaraDataInterface>(Niagara, OverrideName))
		Interface->SetData(BoidCount, PositionBuffer, DirectionBuffer);
}

// Reactivates the Niagara component whenever values are changed in the editor.
void UGPUDrawComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Niagara->Activate(true);
}
