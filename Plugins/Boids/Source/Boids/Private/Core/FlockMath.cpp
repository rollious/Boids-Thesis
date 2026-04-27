#include "Core/FlockMath.h"
#include "Core/BoidConstants.h"

FVector3f FFlockMath::CalculateAlignment(const uint32 Index, const TArray<FVector3f>& Velocities, TArray<uint32>& InNeighbors, const float Strength)
{
	const int32 NeighborAmount = InNeighbors.Num();
	if (NeighborAmount == 0) return FVector3f::ZeroVector;
	
	int32 NeighborIndex = 0;
	FVector3f AlignmentForce = FVector3f::ZeroVector;

	VectorRegister4f Sum = VectorZero();

	for (; NeighborIndex + 3 < NeighborAmount; NeighborIndex += 4)
	{
		const uint32 N0 = InNeighbors[NeighborIndex + 0];
		const uint32 N1 = InNeighbors[NeighborIndex + 1];
		const uint32 N2 = InNeighbors[NeighborIndex + 2];
		const uint32 N3 = InNeighbors[NeighborIndex + 3];

		VectorRegister4f V0 = MakeVectorRegisterFloat(Velocities[N0].X, Velocities[N0].Y, Velocities[N0].Z,0.0f);
		VectorRegister4f V1 = MakeVectorRegisterFloat(Velocities[N1].X, Velocities[N1].Y, Velocities[N1].Z,0.0f);
		VectorRegister4f V2 = MakeVectorRegisterFloat(Velocities[N2].X, Velocities[N2].Y, Velocities[N2].Z,0.0f);
		VectorRegister4f V3 = MakeVectorRegisterFloat(Velocities[N3].X, Velocities[N3].Y, Velocities[N3].Z,0.0f);

		Sum = VectorAdd(Sum, VectorAdd(VectorAdd(V0, V1), VectorAdd(V2, V3)));
	}
	
	float Out[4]; // Number of lanes of the register.
	VectorStoreAligned(Sum, Out);
	AlignmentForce += FVector3f(Out[0], Out[1], Out[2]);
	
	for (; NeighborIndex < NeighborAmount; NeighborIndex++)	AlignmentForce += Velocities[InNeighbors[NeighborIndex]];
    	
	AlignmentForce /= NeighborAmount;
	return (AlignmentForce - Velocities[Index]) * Strength;
}

FVector3f FFlockMath::CalculateCohesion(const uint32 Index, const TArray<FVector3f>& Positions, TArray<uint32>& InNeighbors, const float Strength)
{
	const int32 NeighborAmount = InNeighbors.Num();
	if (NeighborAmount == 0) return FVector3f::ZeroVector;

	int32 NeighborIndex = 0;
	FVector3f CohesionForce = FVector3f::Zero();
	
	VectorRegister4f Sum = VectorZero();
	for (; NeighborIndex + 3 < NeighborAmount; NeighborIndex += 4)
	{
		const uint32 N0 = InNeighbors[NeighborIndex + 0];
		const uint32 N1 = InNeighbors[NeighborIndex + 1];
		const uint32 N2 = InNeighbors[NeighborIndex + 2];
		const uint32 N3 = InNeighbors[NeighborIndex + 3];
		
		VectorRegister4f V0 = MakeVectorRegisterFloat(Positions[N0].X, Positions[N0].Y, Positions[N0].Z, 0.f);
		VectorRegister4f V1 = MakeVectorRegisterFloat(Positions[N1].X, Positions[N1].Y, Positions[N1].Z, 0.f);
		VectorRegister4f V2 = MakeVectorRegisterFloat(Positions[N2].X, Positions[N2].Y, Positions[N2].Z, 0.f);
		VectorRegister4f V3 = MakeVectorRegisterFloat(Positions[N3].X, Positions[N3].Y, Positions[N3].Z, 0.f);

		Sum = VectorAdd(Sum, VectorAdd(VectorAdd(V0, V1), VectorAdd(V2, V3)));
	}
	
	float Out[4];
	VectorStoreAligned(Sum, Out); 
	CohesionForce += FVector3f(Out[0], Out[1], Out[2]);
	
	for (; NeighborIndex < NeighborAmount; NeighborIndex++)	CohesionForce += Positions[InNeighbors[NeighborIndex]];

	CohesionForce /= NeighborAmount;
	return (CohesionForce - Positions[Index]) * Strength;
}

FVector3f FFlockMath::CalculateSeparation(const uint32 Index, const TArray<FVector3f>& Positions, const TArray<uint32>& InNeighbors, const float Strength)
{
	const int32 NeighborAmount = InNeighbors.Num();
	if (NeighborAmount == 0) return FVector3f::ZeroVector;
	
	int32 NeighborIndex = 0;
	FVector3f SeparationForce = FVector3f::ZeroVector;

	VectorRegister4f Sum = VectorZero();
	const VectorRegister4f Position = MakeVectorRegisterFloat(Positions[Index].X, Positions[Index].Y, Positions[Index].Z,0.0f);
	for (; NeighborIndex + 3 < NeighborAmount; NeighborIndex += 4)
	{
		// Correctly map neighbor indices
		const uint32 N0 = InNeighbors[NeighborIndex + 0];
		const uint32 N1 = InNeighbors[NeighborIndex + 1];
		const uint32 N2 = InNeighbors[NeighborIndex + 2];
		const uint32 N3 = InNeighbors[NeighborIndex + 3];

		VectorRegister4f V0 = MakeVectorRegisterFloat(Positions[N0].X, Positions[N0].Y, Positions[N0].Z,0.0f);
		VectorRegister4f V1 = MakeVectorRegisterFloat(Positions[N1].X, Positions[N1].Y, Positions[N1].Z,0.0f);
		VectorRegister4f V2 = MakeVectorRegisterFloat(Positions[N2].X, Positions[N2].Y, Positions[N2].Z,0.0f);
		VectorRegister4f V3 = MakeVectorRegisterFloat(Positions[N3].X,	Positions[N3].Y, Positions[N3].Z,0.0f);

		// SUBTRACTION
		V0 = VectorSubtract(Position, V0);
		V1 = VectorSubtract(Position, V1);
		V2 = VectorSubtract(Position, V2);
		V3 = VectorSubtract(Position, V3);
		
		// NORMALIZATION
		VectorRegister4f LenSq0 = VectorDot3(V0, V0);
		VectorRegister4f LenSq1 = VectorDot3(V1, V1);
		VectorRegister4f LenSq2 = VectorDot3(V2, V2);
		VectorRegister4f LenSq3 = VectorDot3(V3, V3);
		
		const VectorRegister4f MinLen = MakeVectorRegisterFloat(UE_SMALL_NUMBER, UE_SMALL_NUMBER, UE_SMALL_NUMBER, UE_SMALL_NUMBER);
		LenSq0 = VectorMax(LenSq0, MinLen);
		LenSq1 = VectorMax(LenSq1, MinLen);
		LenSq2 = VectorMax(LenSq2, MinLen);
		LenSq3 = VectorMax(LenSq3, MinLen);
		
		V0 = VectorMultiply(V0, VectorReciprocalSqrt(LenSq0));
		V1 = VectorMultiply(V1, VectorReciprocalSqrt(LenSq1));
		V2 = VectorMultiply(V2, VectorReciprocalSqrt(LenSq2));
		V3 = VectorMultiply(V3, VectorReciprocalSqrt(LenSq3));

		Sum = VectorAdd(Sum, VectorAdd(VectorAdd(V0, V1), VectorAdd(V2, V3)));
	}
	
	float Out[4]; // Number of lanes of the register.
	VectorStoreAligned(Sum, Out);
	SeparationForce += FVector3f(Out[0], Out[1], Out[2]);
	
	for (; NeighborIndex < NeighborAmount; NeighborIndex++)
	{
		const FVector3f SeparationDir = Positions[Index] - Positions[InNeighbors[NeighborIndex]];
		SeparationForce += SeparationDir / FMath::Max(SeparationDir.SquaredLength(), UE_KINDA_SMALL_NUMBER);
	}
	
	return SeparationForce * Strength;
}

FVector3f FFlockMath::CalculateTarget(const FVector3f Position, const FVector3f Target,
	const float Strength, const FBoidMovement& Settings)
{
	FVector3f TargetDirection = FVector3f(0, 0, 0);
	const float Length = FVector3f::Dist(Target, Position);
	
	if (Length > Settings.MaxDistanceFromTarget) TargetDirection = (Target - Position).GetSafeNormal(); // Steer toward target
	if (Length < Settings.MinDistanceFromTarget) TargetDirection = (Position - Target).GetSafeNormal(); // Steer away from target
	
	return TargetDirection * Length * Strength;
}
