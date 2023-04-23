// Bullet Penetration System. All rights reserved

#include "BulletPenetrationComponent.h"
#include "ImpactSurfaceInfo.h"
#include "PenetrationPhysMaterial.h"
#include "Components/DecalComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
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
void UBulletPenetrationComponent::Shoot(const FVector ShootLocation, const FVector ShootDirection,
	TArray<AActor*> IgnoreActors, AController* DamageInstigator)
{
	FCurrentBulletInfo LocalBulletInfo;
	LocalBulletInfo.BulletPenetration = BulletPenetration;
	LocalBulletInfo.BulletDistance = 0.0f;
	LocalBulletInfo.BulletDamage = BulletDamage;
	IgnoreActors.AddUnique(GetOwner()); // Ignore weapon
	HitLogic(ShootLocation,ShootDirection*ShootDistance,LocalBulletInfo,IgnoreActors,DamageInstigator);
}

bool UBulletPenetrationComponent::BulletTrace(const FVector ShootLocation, const FVector ShootVector,
	FHitResult& OutHit, TArray<AActor*> IgnoreActors) const
{
	const FVector Start = ShootLocation;
	const FVector End = Start + ShootVector;
	if(!GetWorld()) return false;
	bool LocalBlockingHit;
	switch(BulletTraceType)
	{
		case EBulletTraceType::Line:
			LocalBlockingHit = UKismetSystemLibrary::LineTraceSingle(
				GetWorld(),
				Start,
				End,
				UEngineTypes::ConvertToTraceType(BulletShootChannel),
				true,
				IgnoreActors,
				bShowVisualDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
				OutHit,
				true,
				FLinearColor::Red,
				FLinearColor::Green,
				ShowVisualDebugTime);
			break;
		case EBulletTraceType::Sphere:
			LocalBlockingHit = UKismetSystemLibrary::SphereTraceSingle(
				GetWorld(),
				Start,
				End,
				BulletRadius,
				UEngineTypes::ConvertToTraceType(BulletShootChannel),
				true,
				IgnoreActors,
				bShowVisualDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
				OutHit,
				true,
				FLinearColor::Red,
				FLinearColor::Green,
				ShowVisualDebugTime);
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Bullet trace is invalid!!!"));
			return false;
	}
	return LocalBlockingHit;
}

bool UBulletPenetrationComponent::PenetrationTrace(const FVector Direction, const FVector EnterLocation,
	AActor* HitActor, TArray<AActor*> IgnoreActors, FVector& PenetrationLocation) const
{
	PenetrationLocation = FVector::Zero();
	if(!HitActor) return false;
	// Find largest hit actor side
	FVector Origin;
	FVector ActorBoxExtent;
	HitActor->GetActorBounds(true,Origin,ActorBoxExtent,true); // Get Actor Bounds
	float LargestActorSide; // Length largest side
	int32 LocalInt;
	const TArray<float> MaxSideArray = {static_cast<float>(ActorBoxExtent.X),static_cast<float>(ActorBoxExtent.Y),static_cast<float>(ActorBoxExtent.Z)}; 
	UKismetMathLibrary::MaxOfFloatArray(MaxSideArray,LocalInt,LargestActorSide);
	const FVector Start = EnterLocation + LargestActorSide * 2.05f * Direction;
	const FVector End = EnterLocation;
	TArray<FHitResult> OutHits;
	// Do Penetration Test Trace
	switch(BulletTraceType)
	{
	case EBulletTraceType::Line:
		UKismetSystemLibrary::LineTraceMulti(
			GetWorld(),
			Start,
			End,
			UEngineTypes::ConvertToTraceType(BulletShootChannel),
			true,
			IgnoreActors,
			bShowVisualDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
			OutHits,
			true,
			FLinearColor::Blue,
			FLinearColor::Yellow,
			ShowVisualDebugTime);
		break;
	case EBulletTraceType::Sphere:
		UKismetSystemLibrary::SphereTraceMulti(
			GetWorld(),
			Start,
			End,
			BulletRadius,
			UEngineTypes::ConvertToTraceType(BulletShootChannel),
			true,
			IgnoreActors,
			bShowVisualDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
			OutHits,
			true,
			FLinearColor::Blue,
			FLinearColor::Yellow,
			ShowVisualDebugTime);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Bullet trace is invalid!!!"));
		return false;
	}
	if(OutHits.IsEmpty()) return false;
	for(int i = OutHits.Num()-1; i<0;--i)
	{
		const FHitResult LocalHit = OutHits[i];
		if(LocalHit.GetActor() != HitActor) continue;
		const float PenetrationDistance = UKismetMathLibrary::Vector_Distance(LocalHit.ImpactPoint,EnterLocation); // Get Penetration Distance
		if(!IsCanPenetraceDistance(PenetrationDistance,LocalHit.PhysMaterial)) return false;
		auto LocalImpactInfo = ImpactInfo->ImpactInfo[UGameplayStatics::GetSurfaceType(LocalHit)]; // Get ImpactInfo
		SpawnDecal(LocalHit,LocalImpactInfo.ImpactDecal);
		/*
		Penetrate location move on 4 units from start to stop hit bags
		If Bullet trace == Sphere - add move on Radius + 4 units
		*/
		PenetrationLocation = LocalHit.ImpactPoint + LocalHit.ImpactNormal*4.0f +
		(BulletTraceType == EBulletTraceType::Sphere ? 0.0f : BulletRadius);
		return true;
	}
	return false;
}

void UBulletPenetrationComponent::HitLogic(const FVector ShootLocation, const FVector ShootVector, FCurrentBulletInfo& NewBulletInfo, TArray<AActor*> IgnoreActors, AController* DamageInstigator)
{
	FHitResult HitResult;
	if(!(BulletTrace(ShootLocation,ShootVector,HitResult,IgnoreActors) && HitResult.GetActor())) return;
	
	NewBulletInfo.BulletDistance = NewBulletInfo.BulletDistance + HitResult.Distance; // Calculate Distance
	NewBulletInfo.BulletDamage = CalculateDamage(NewBulletInfo); // Calculate Damage
	NewBulletInfo.BulletPenetration = CalculatePenetrationByDistance(NewBulletInfo) -
		CalculatePenetrationBySurface(NewBulletInfo, HitResult.PhysMaterial); // Calculate Penetration
	ShowDebugBulletInfo(NewBulletInfo);
	const FVector ShootDirection = UKismetMathLibrary::FindLookAtRotation(HitResult.TraceStart,HitResult.TraceEnd).Vector();
	UGameplayStatics::ApplyPointDamage(HitResult.GetActor(),
		NewBulletInfo.BulletDamage,
		ShootDirection,
		HitResult,
		DamageInstigator,
		GetOwner(),
		DamageTypeClass); // Damage Logic
	HitDamageEvent.Broadcast(HitResult,DamageInstigator,GetOwner(),HitResult.ImpactPoint,
		NewBulletInfo.BulletDamage,NewBulletInfo.BulletPenetration,NewBulletInfo.BulletDistance); // Broadcast Hit Event 
	SpawnVFX(HitResult,NewBulletInfo.BulletPenetration<0);
	if(NewBulletInfo.BulletPenetration>=0)
	{
		FVector PenetrationSpawnLocation;
		const bool Success = PenetrationTrace(
			ShootDirection,
			HitResult.ImpactPoint,
			HitResult.GetActor(),
			IgnoreActors,
			PenetrationSpawnLocation);
		const float NewShootDistance = ShootDistance-NewBulletInfo.BulletDistance;
		if(!(NewShootDistance>0.0f && Success)) return;
		HitLogic(
			PenetrationSpawnLocation,
			ShootDirection * NewShootDistance,
			NewBulletInfo,
			IgnoreActors,
			DamageInstigator
			); // New bullet shoot distance = StartDistance - ShootDistance
		ShowDebugHitSphere(HitResult.ImpactPoint, FLinearColor::Green); // Debug when hit
	}
	else ShowDebugHitSphere(HitResult.ImpactPoint, FLinearColor::Red); // Debug when last hit
	MakeImpulseAtImpactLocation(HitResult,NewBulletInfo.BulletDamage*ImpulseStrengthMultiplier); // Make Impulse after detect new penetration location
}

// Service Logic
float UBulletPenetrationComponent::CalculateDamage(FCurrentBulletInfo& BulletInfo) const
{
	const float LocalPenetration = CalculatePenetrationByDistance(BulletInfo);
	const float LocalDamageByPenetration = UKismetMathLibrary::MapRangeClamped(LocalPenetration,
		0.0f,
		BulletPenetration,
		DamageDecreaseMultiplier,
		1.0f);
	const float LocalDamageByDistance = UKismetMathLibrary::MapRangeClamped(BulletInfo.BulletDistance,
		150.0f, // MinDistance To Start Calculate
		DamageFalloffDistance,
		BulletInfo.BulletDamage,
		BulletInfo.BulletDamage/4.0f);
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
	if(!LocalPhysMaterial) return BulletInfo.BulletPenetration+0.1f; // If no valid cast can't penetrate
	return LocalPhysMaterial->PenetrationInfo.PenetrationDecrease < 0 ?
		BulletInfo.BulletPenetration+0.1f :
	LocalPhysMaterial->PenetrationInfo.PenetrationDecrease;
	// If PenetrationDecrease < 0 bullet will be stopped
}

void UBulletPenetrationComponent::MakeImpulseAtImpactLocation(const FHitResult HitResult, const float ImpulseStrength) const
{
	if(!(HitResult.bBlockingHit && HitResult.GetComponent() && HitResult.GetComponent()->IsSimulatingPhysics())) return;
	const FVector Impulse = UKismetMathLibrary::FindLookAtRotation(HitResult.TraceStart,HitResult.TraceEnd).Vector() * ImpulseStrength;
	HitResult.GetComponent()->AddImpulseAtLocation(Impulse,HitResult.ImpactPoint,HitResult.BoneName);
}

bool UBulletPenetrationComponent::IsCanPenetraceDistance(const float PenetrateDistance, const TWeakObjectPtr<UPhysicalMaterial> PhysMaterial) const
{
	auto LocalPhysMaterial = Cast<UPenetrationPhysMaterial>(PhysMaterial);
	if(!LocalPhysMaterial) return false;
	return PenetrateDistance <= LocalPhysMaterial->PenetrationInfo.MaxPenetrationDistance;
}

// VFX
void UBulletPenetrationComponent::SpawnVFX(const FHitResult HitResult, const bool LastHit = false) const
{
	if(!ImpactInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Impact Info is invalid!!!"));
		return;
	}
	auto LocalImpactInfo = ImpactInfo->ImpactInfo[UGameplayStatics::GetSurfaceType(HitResult)]; // If last hit return all last VFX
	SpawnDecal(HitResult,LastHit ? LocalImpactInfo.ImpactLastDecal : LocalImpactInfo.ImpactDecal);
	SpawnNiagara(HitResult, LastHit ? LocalImpactInfo.ImpactLastNiagara : LocalImpactInfo.ImpactNiagara);
	SpawnSound(HitResult, LastHit ? LocalImpactInfo.ImpactLastSound : LocalImpactInfo.ImpactSound);
}

void UBulletPenetrationComponent::SpawnDecal(const FHitResult HitResult, UMaterialInterface* ImpactDecal) const
{
	const float DecalSize = UKismetMathLibrary::RandomFloatInRange(6.0f,7.0f);
	auto Decal = UGameplayStatics::SpawnDecalAttached(
		ImpactDecal,
		FVector(DecalSize,DecalSize,DecalSize),
		HitResult.GetComponent(),
		HitResult.BoneName,
		HitResult.ImpactPoint,
		UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal),
		EAttachLocation::KeepWorldPosition,
		600.0f
		);
	if(!Decal) return;
	Decal->SetFadeScreenSize(0.002f); // Set fade screen size
	Decal->AddRelativeRotation(FRotator(UKismetMathLibrary::RandomFloatInRange(-90.0f,90.0f),0.0f,0.0f)); // Add decal random rotation
}

void UBulletPenetrationComponent::SpawnNiagara(const FHitResult HitResult, UNiagaraSystem* ImpactNiagara) const
{
	if(!GetWorld()) return;
	auto Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ImpactNiagara,
		HitResult.ImpactPoint,
		UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal),
		FVector::One()
		);
}

void UBulletPenetrationComponent::SpawnSound(const FHitResult HitResult, USoundBase* ImpactSound) const
{
	if(!GetWorld()) return;
	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		ImpactSound,
		HitResult.ImpactPoint,
		UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal)
		);
}

// Debug
void UBulletPenetrationComponent::ShowDebugHitSphere(const FVector SpawnLocation, const FLinearColor SphereColor = FLinearColor::Green) const
{
	if(!GetWorld() || !bShowVisualDebug) return;
	UKismetSystemLibrary::DrawDebugSphere(
		GetWorld(),
		SpawnLocation,
		20,
		12,
		SphereColor,
		ShowVisualDebugTime
		);
}

void UBulletPenetrationComponent::ShowDebugBulletInfo(const FCurrentBulletInfo BulletInfo) const
{
	if(!GetWorld() || !bShowBulletInfo) return;
	UKismetSystemLibrary::PrintString(
		GetWorld(),
		FString("---------------------------------------\nDistance: " + FString::SanitizeFloat(BulletInfo.BulletDistance) + '\n'
		+ "Penetration: " + FString::SanitizeFloat(BulletInfo.BulletPenetration) + '\n'
		+ "Damage: " + FString::SanitizeFloat(BulletInfo.BulletDamage) +
		"---------------------------------------\n"),
		true,
		true,
		FLinearColor::Green,
		ShowBulletInfoTime
		);
}
