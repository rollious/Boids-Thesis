// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Structs/GPUStructs.h"

#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

#include "GPUDrawComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BOIDS_API UGPUDrawComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGPUDrawComponent();
	
	void InitGameThread(int32 BoidCount, const FDoubleBuffer& PositionBuffer, const FDoubleBuffer& VelocityBuffer) const;

	void UpdateNiagaraBuffer(FName OverrideName, uint32 BoidCount, const TRefCountPtr<FRDGPooledBuffer>& PositionBuffer, const TRefCountPtr<FRDGPooledBuffer>& DirectionBuffer) const;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	void SetNiagaraSystem(const TSoftObjectPtr<UNiagaraSystem>& Sys) { NiagaraSystem = Sys; }
	UNiagaraComponent* GetNiagara() { return Niagara; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIAGARA")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;
	
private:
	UPROPERTY(Transient/*, EditDefaultsOnly, Category = "NIAGARA"*/)
	TObjectPtr<UNiagaraComponent> Niagara = nullptr;
	
	FMatrix BoundsMatrix;
};
