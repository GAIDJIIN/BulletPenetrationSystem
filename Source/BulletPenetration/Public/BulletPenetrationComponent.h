// Bullet Penetration System. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BulletPenetrationComponent.generated.h"

class UImpactSurfaceInfo;
class UNiagaraSystem;

// Use struct instead of object
struct FCurrentBulletInfo
{
	float BulletPenetration = 0.0f;
	float BulletDistance = 0.0f;
	float BulletDamage = 0.0f;
};

UENUM(BlueprintType)
enum class EBulletTraceType : uint8
{
	Line,
	Sphere,
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BULLETPENETRATION_API UBulletPenetrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBulletPenetrationComponent();
	// Blueprint
	// Function
	UFUNCTION(BlueprintCallable)
		void Shoot(const FVector ShootLocation, const FVector ShootDirection, TArray<AActor*> IgnoreActors, AController* DamageInstigator); // Main function shoot
protected:
	virtual void BeginPlay() override;

private:
	// Blueprint
	// Variables
	// Bullet Info
	UPROPERTY(EditAnywhere,Category="Bullet Info|Penetration Info",meta=(ClampMin="0.0", ToolTip = "Bullet penetration force", AllowPrivateAccess))
		float BulletPenetration = 1.0f; // Bullet penetration force
	UPROPERTY(EditAnywhere,Category="Bullet Info|Penetration Info",meta=(ClampMin="0.0", ToolTip = "Max extremum distance penetration", AllowPrivateAccess))
		float PenetrationFalloffDistance = 2500.0f; // Max extremum distance penetration		
	UPROPERTY(EditAnywhere,Category="Bullet Info|Penetration Info",meta=(ClampMin="0.0", ClampMax="1.0",ToolTip = "Bullet penetration decrease by distance parameter", AllowPrivateAccess))
		float RatioPenetration = 0.5f; // Bullet penetration decrease by distance parameter
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Info",meta=(ClampMin="0.0", ToolTip = "Bullet shoot distance", AllowPrivateAccess))
		float ShootDistance = 10000.0f; // Bullet shoot distance
	UPROPERTY(EditAnywhere,Category="Bullet Info|Damage Info",meta=(ClampMin="0.0", ToolTip = "Bullet damage", AllowPrivateAccess))
		float BulletDamage = 10.0f; // Bullet damage
	UPROPERTY(EditAnywhere,Category="Bullet Info|Damage Info",meta=(ClampMin="0.0", ToolTip = "Max extremum distance damage", AllowPrivateAccess))
		float DamageFalloffDistance = 2500.0f; // Max extremum distance damage
	UPROPERTY(EditAnywhere,Category="Bullet Info|Damage Info",meta=(ClampMin="0.0", ToolTip = "Max extremum distance damage", AllowPrivateAccess))
		float ImpulseStrengthMultiplayer = 40.0f;
	UPROPERTY(EditAnywhere,Category="Bullet Info|Damage Info",meta=(ToolTip = "Damage type class", AllowPrivateAccess))
		TSubclassOf<UDamageType> DamageTypeClass = nullptr; // Damage type class

	// Bullet trace type
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Trace Channel",meta=(ToolTip = "Bullet Trace Type (Line,Box,Sphere,Capsule)", AllowPrivateAccess))
		EBulletTraceType BulletTraceType = EBulletTraceType::Line; // Bullet Trace Type (Line,Box,Sphere,Capsule)
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Trace Channel",meta=(ClampMin="0.0",ToolTip = "Sphere or Capsule radius", AllowPrivateAccess, EditCondition = "BulletTraceType == EBulletTraceType::Sphere", EditConditionHides))
		float BulletRadius = 0.0f;
	
	// Penetration Info
	UPROPERTY(EditAnywhere,Category="Bullet Info|Impact Info",meta=(ToolTip = "Impact Info"))
		TObjectPtr<UImpactSurfaceInfo> ImpactInfo;
	
	// Collision Channels Info
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Trace Channel",meta=(ToolTip = "Bullet Shoot Channel", AllowPrivateAccess))
		TEnumAsByte<ECollisionChannel> BulletShootChannel = ECC_Visibility; // Bullet Shoot Channel
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Trace Channel",meta=(ToolTip = "Penetration Shoot Channel", AllowPrivateAccess))
		TEnumAsByte<ECollisionChannel> PenetrationShootChannel = ECC_Visibility; // Penetration Shoot Channel

	// Debug
	UPROPERTY(EditAnywhere,Category="Bullet Info|Debug",meta=(ToolTip = "Show bullet current info", AllowPrivateAccess))
		bool bShowBulletInfo = false; // Show bullet current info
	UPROPERTY(EditAnywhere,Category="Bullet Info|Debug",meta=(ToolTip = "Time to show", AllowPrivateAccess, EditCondition = "bShowBulletInfo"))
		float ShowBulletInfoTime = 5.0f;
	UPROPERTY(EditAnywhere,Category="Bullet Info|Debug",meta=(ToolTip = "Show debug traces", AllowPrivateAccess)) 
		bool bShowVisualDebug = false; // Show debug traces
	UPROPERTY(EditAnywhere,Category="Bullet Info|Debug",meta=(ToolTip = "Time to show", AllowPrivateAccess, EditCondition = "bShowVisualDebug"))
		float ShowVisualDebugTime = 5.0f;
	
	// C++
	// Variables
	
	// Function
	// Debug
	FString GetDebugInfo(FCurrentBulletInfo& BulletInfo) const;
	
	// Penetration Logic
	bool BulletTrace(const FVector ShootLocation, const FVector ShootVector, FHitResult& OutHit, TArray<AActor*> IgnoreActors) const; // Do Bullet Trace
	bool PenetrationTrace(const FVector Direction, const FVector EnterLocation, AActor* HitActor, FVector& PenetrationLocation) const; // Do Penetration Trace
	void HitLogic(const FVector ShootLocation, const FVector ShootVector, FCurrentBulletInfo& NewBulletInfo, TArray<AActor*> IgnoreActors, AController* DamageInstigator); // Core Hit Logic

	// Service Logic
	float CalculateDamage(FCurrentBulletInfo& BulletInfo) const; // Calculate Damage by Penetration and Distance
	float CalculatePenetrationByDistance(FCurrentBulletInfo& BulletInfo) const; // Calculate Penetration by Distance
	float CalculatePenetrationBySurface(FCurrentBulletInfo& BulletInfo,const TWeakObjectPtr<UPhysicalMaterial> PhysMaterial) const; // Calculate Penetration by Surface
	void MakeImpulseAtImpactLocation(const FHitResult HitResult, const float ImpulseStrength) const; // Impulse Strength = BulletDamage * ImpulseStrengthMultiplayer

	// VFX
	void SpawnVFX(const FHitResult HitResult, const bool LastHit) const;
	void SpawnDecal(const FHitResult HitResult, UMaterialInterface* ImpactDecal) const;
	void SpawnNiagara(const FHitResult HitResult, UNiagaraSystem* ImpactNiagara) const;
	void SpawnSound(const FHitResult HitResult, USoundBase* ImpactSound) const;
};
