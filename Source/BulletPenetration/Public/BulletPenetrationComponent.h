// Bullet Penetration System. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BulletPenetrationComponent.generated.h"

class UImpactSurfaceInfo;

// Use struct instead of object
USTRUCT(BlueprintType)
struct FCurrentBulletInfo
{
	GENERATED_USTRUCT_BODY()
		float BulletPenetration = 0.0f;
		float BulletDistance = 0.0f;
		float BulletDamage = 0.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BULLETPENETRATION_API UBulletPenetrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBulletPenetrationComponent();
	// Blueprint
	// Function
	UFUNCTION(BlueprintCallable)
		void Shoot(const FVector ShootLocation, const FVector ShootDirection, TArray<AActor*> IgnoreActors); // Main function shoot
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
	UPROPERTY(EditAnywhere,Category="Bullet Info|Damage Info",meta=(ToolTip = "Damage type class", AllowPrivateAccess))
		TObjectPtr<UDamageType> DamageTypeClass = nullptr; // Damage type class
	
	// Penetration Info
	UPROPERTY(EditAnywhere,Category="Impact Info",meta=(ToolTip = "Impact Info"))
		TObjectPtr<UImpactSurfaceInfo> ImpactInfo;
	
	// Collision Channels Info
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Trace Channel",meta=(ToolTip = "Bullet Shoot Channel", AllowPrivateAccess))
		TEnumAsByte<ECollisionChannel> BulletShootChannel = ECC_Visibility; // Bullet Shoot Channel
	UPROPERTY(EditAnywhere,Category="Bullet Info|Bullet Trace Channel",meta=(ToolTip = "Penetration Shoot Channel", AllowPrivateAccess))
		TEnumAsByte<ECollisionChannel> PenetrationShootChannel = ECC_Visibility; // Penetration Shoot Channel

	// Debug
	UPROPERTY(EditAnywhere,Category="Debug",meta=(ToolTip = "Show bullet current info", AllowPrivateAccess))
		bool bShowBulletInfo = false; // Show bullet current info
	UPROPERTY(EditAnywhere,Category="Debug",meta=(ToolTip = "Time to show", AllowPrivateAccess, EditCondition = "bShowBulletInfo"))
		float ShowBulletInfoTime = 5.0f;
	UPROPERTY(EditAnywhere,Category="Debug",meta=(ToolTip = "Show debug traces", AllowPrivateAccess)) 
		bool bShowVisualDebug = false; // Show debug traces
	UPROPERTY(EditAnywhere,Category="Debug",meta=(ToolTip = "Time to show", AllowPrivateAccess, EditCondition = "bShowVisualDebug"))
		float ShowVisualDebugTime = 5.0f;
	
	// C++
	// Variables
	
	// Function
	void ShowDebug(float DebugTime);

	// Penetration Logic
	void BulletTrace(const FVector ShootLocation, const FVector ShootVector, FCurrentBulletInfo& NewBulletInfo, TArray<AActor*> IgnoreActors); // Do Bullet Trace
	void HitLogic(const FHitResult& HitResult, FCurrentBulletInfo& NewBulletInfo); // Core Hit Logic

	// Service Logic
	float CalculateDamage(FCurrentBulletInfo& BulletInfo) const; // Calculate Damage by Penetration and Distance
	float CalculatePenetrationByDistance(FCurrentBulletInfo& BulletInfo) const; // Calculate Penetration by Distance
	float CalculatePenetrationBySurface(FCurrentBulletInfo& BulletInfo,const TWeakObjectPtr<UPhysicalMaterial> PhysMaterial) const; // Calculate Penetration by Surface
};
