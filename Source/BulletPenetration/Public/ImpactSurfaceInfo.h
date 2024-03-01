// Bullet Penetration System. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ImpactSurfaceInfo.generated.h"

class UNiagaraSystem;

USTRUCT()
struct FImpactVFX
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
		TSoftObjectPtr<UMaterialInterface> ImpactDecal;
	UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
		TSoftObjectPtr<UMaterialInterface> ImpactLastDecal;
	UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
		TSoftObjectPtr<UNiagaraSystem> ImpactNiagara;
	UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
		TSoftObjectPtr<UNiagaraSystem> ImpactLastNiagara;
	UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
		TSoftObjectPtr<USoundBase> ImpactSound;
	UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
		TSoftObjectPtr<USoundBase> ImpactLastSound;
};

UCLASS()
class BULLETPENETRATION_API UImpactSurfaceInfo : public UDataAsset
{
	GENERATED_BODY()
	public:
		UPROPERTY(EditAnywhere, Category="Impact Info|VFX")
			TMap<TEnumAsByte<EPhysicalSurface>,FImpactVFX> ImpactInfo;
};
