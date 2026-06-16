// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/WasteActor_Base.h"
#include "CCD_FreezeGrenade.generated.h"

class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class CCD_API ACCD_FreezeGrenade : public AWasteActor_Base
{
	GENERATED_BODY()
	
public:	
	ACCD_FreezeGrenade();
	void Launch(FVector LaunchDirection);

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Design | Sound")
	TObjectPtr<USoundBase> ExplosionSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Design | Effect")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Design | Grenade")
	float FreezeRadius = 500.0f;
	
	UFUNCTION()
	void OnGrenadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayExplosionEffects(FVector Location);
	
	void Detonate();

private:
	bool bIsLaunched = false;
};
