// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/InteractInterface.h"
#include "WashableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UWashableComponent : public UActorComponent, public IInteractInterface
{
	GENERATED_BODY()

public:	
	UWashableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(ReplicatedUsing = OnRep_WashHealth, EditAnywhere, Category = "Status")
	float WashHealth = 100.f;
	UFUNCTION()
	void OnRep_WashHealth();
	
	// 같은 액터에 있는 점수 컴포넌트 참조
	class UProgressComponent* ProgressComp;

public:	
	// Implement Interact interface 오버라이드
	virtual void Interact_Implementation(AActor* Interactor) override;
	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	float getWashHealthRatio() const { return WashHealth / 100.f; }
	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	void TakeWashDamage(float DamageAmount);
};