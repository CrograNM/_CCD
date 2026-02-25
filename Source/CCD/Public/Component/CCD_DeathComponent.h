
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCD_DeathComponent.generated.h"

// 이벤트를 수신할 클래스들을 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AController*, Causer);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CCD_API UCCD_DeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCD_DeathComponent();
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 외부(전투 로직 등)에서 호출할 데미지 함수
	UFUNCTION(BlueprintCallable, Category = "Death")
	void ProcessDamage(float Damage, AController* Instigator);

	/** --- Getters --- */
	bool IsDead() const { return bIsDead; }
	float GetHealth() const { return Health; }

	/** --- Delegates --- */
	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, Category = "Status")
	float Health = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_IsDead();

	void HandleDeath(AController* Killer);
};
