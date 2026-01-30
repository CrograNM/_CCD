
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WashableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UWashableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWashableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(ReplicatedUsing = OnRep_WashHealth, EditAnywhere, Category = "Status")
	float WashHealth = 100.f;
	UFUNCTION()
	void OnRep_WashHealth();
	
	// 같은 액터에 있는 점수 컴포넌트 참조
	class UProgressComponent* ProgressComp;

public:	
	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	float getWashHealthRatio() const { return WashHealth / 100.f; }
	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	void TakeWashDamage(float DamageAmount);
};