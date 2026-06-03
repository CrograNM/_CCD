// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/WasteActor_Base.h"
#include "CCD_FreezeGrenade.generated.h"

class UProjectileMovementComponent;

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
	
	UFUNCTION()
	void OnGrenadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	void Detonate(AActor* TargetActor);

private:
	bool bIsLaunched = false;
};
