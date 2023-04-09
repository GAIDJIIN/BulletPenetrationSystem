// Bullet Penetration System. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PenetrationPhysMaterial.generated.h"

// Penetration Info By Surface
USTRUCT()
struct FPenetrationInfo
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere,Category="Penetration Info")
		float PenetrationDecrease = 0.0f; // Decrease Penetration by Surface
	UPROPERTY(EditAnywhere,Category="Penetration Info")
		float MaxPenetrationDistance = 0.0f; // Max Penetration Distance 
};

UCLASS()
class BULLETPENETRATION_API UPenetrationPhysMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
	public:
		UPROPERTY(EditAnywhere,Category="Penetration Info")
			FPenetrationInfo PenetrationInfo;
};
