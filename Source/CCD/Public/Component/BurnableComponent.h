// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/InteractInterface.h"
#include "BurnableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UBurnableComponent : public UActorComponent, public IInteractInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBurnableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_BurnHealth, EditAnywhere, Category = "Status")
	float BurnHealth = 100.f;
	UFUNCTION()
	void OnRep_BurnHealth();
	
	// 같은 액터에 있는 점수 컴포넌트 참조
	class UProgressComponent* ProgressComp;
	
public:
	// 캐릭터가 상호작용(E키) 눌렀을 때 호출됨
	virtual void Interact_Implementation(AActor* Interactor) override;	
	
	UFUNCTION(BlueprintCallable, Category = "Burnable")
	void TakeBurnDamage(float DamageAmount);
};
