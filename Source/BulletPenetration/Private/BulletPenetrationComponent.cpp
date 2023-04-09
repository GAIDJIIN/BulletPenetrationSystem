// Bullet Penetration System. All rights reserved

#include "BulletPenetrationComponent.h"

#include "PenetrationPhysMaterial.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UBulletPenetrationComponent::UBulletPenetrationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UBulletPenetrationComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Penetration Logic
void UBulletPenetrationComponent::Shoot(const FVector ShootLocation, const FVector ShootDirection, TArray<AActor*> IgnoreActors)
{
	FCurrentBulletInfo LocalBulletInfo;
	LocalBulletInfo.BulletPenetration = BulletPenetration;
	LocalBulletInfo.BulletDistance = 0.0f;
	LocalBulletInfo.BulletDamage = BulletDamage;
	IgnoreActors.Add(GetOwner()); // Ignore weapon
	BulletTrace(ShootLocation,ShootDirection*ShootDistance,LocalBulletInfo,IgnoreActors);
}

void UBulletPenetrationComponent::BulletTrace(const FVector ShootLocation, const FVector ShootVector,
	FCurrentBulletInfo& NewBulletInfo, TArray<AActor*> IgnoreActors)
{
	const FVector Start = ShootLocation;
	const FVector End = Start + ShootVector;
	if(!GetWorld()) return;
	FHitResult LocalHit;
	bool LocalBlockingHit = UKismetSystemLibrary::LineTraceSingle(
	GetWorld(),
	Start,
	End,
	UEngineTypes::ConvertToTraceType(BulletShootChannel),
	true,
	IgnoreActors,
	bShowVisualDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
	LocalHit,
	true,
	FLinearColor::Red,
	FLinearColor::Green,
	ShowVisualDebugTime
	);
	if(LocalBlockingHit) HitLogic(LocalHit,NewBulletInfo);
}

void UBulletPenetrationComponent::HitLogic(const FHitResult& HitResult, FCurrentBulletInfo& NewBulletInfo)
{
	NewBulletInfo.BulletDistance = NewBulletInfo.BulletDistance + HitResult.Distance; // Calculate Distance
	NewBulletInfo.BulletDamage = CalculateDamage(NewBulletInfo); // Calculate Damage
	NewBulletInfo.BulletPenetration = CalculatePenetrationByDistance(NewBulletInfo) -
		CalculatePenetrationBySurface(NewBulletInfo, HitResult.PhysMaterial); // Calculate Penetration
}

// Service Logic
float UBulletPenetrationComponent::CalculateDamage(FCurrentBulletInfo& BulletInfo) const
{
	const float LocalPenetration = CalculatePenetrationByDistance(BulletInfo);
	const float LocalDamageByPenetration = UKismetMathLibrary::MapRangeClamped(LocalPenetration,
		0.0f,
		BulletPenetration,
		RatioPenetration,
		1.0f);
	const float LocalDamageByDistance = UKismetMathLibrary::MapRangeClamped(BulletInfo.BulletDistance,
		150.0f, // MinDistance To Start Calculate
		DamageFalloffDistance,
		BulletInfo.BulletDamage,
		BulletInfo.BulletDamage/4);
	return LocalDamageByPenetration * LocalDamageByDistance;
}

float UBulletPenetrationComponent::CalculatePenetrationByDistance(FCurrentBulletInfo& BulletInfo) const
{
	const float CalculatedPenetration = UKismetMathLibrary::MapRangeClamped(BulletInfo.BulletDistance,
		250.0f, // MinDistance To Start Calculate
		PenetrationFalloffDistance,
		BulletInfo.BulletPenetration,
		0.0f);
	return CalculatedPenetration;
}

float UBulletPenetrationComponent::CalculatePenetrationBySurface(FCurrentBulletInfo& BulletInfo,
	const TWeakObjectPtr<UPhysicalMaterial> PhysMaterial) const
{
	auto LocalPhysMaterial = Cast<UPenetrationPhysMaterial>(PhysMaterial);
	if(!LocalPhysMaterial) return 0.0f;
	return LocalPhysMaterial->PenetrationInfo.PenetrationDecrease < 0 ?
		BulletInfo.BulletPenetration+0.1f :
	LocalPhysMaterial->PenetrationInfo.PenetrationDecrease;
	// If PenetrationDecrease < 0 bullet will be stopped
}

// Debug
void UBulletPenetrationComponent::ShowDebug(float DebugTime = 0.01f)
{
	// text and traces
}



