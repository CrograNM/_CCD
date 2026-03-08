
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCD_StatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float /*CurrentStamina*/, float /*MaxStamina*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CCD_API UCCD_StatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCD_StatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable)
	void Server_SetSpeed(const bool bNewIsRunning);
	void SetIsRunning(const bool bNewIsRunning);
	
	FOnStaminaChanged OnStaminaChanged;
	float GetCurrentStamina() const { return CurrentStamina; }
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 달리기 / 스태미나 --- */
	UPROPERTY(Replicated)
	bool bIsRunning = false;
	
	UPROPERTY(EditAnywhere, Category = "Stats")
	float RunSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Stats")
	float WalkSpeed = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Stats")
	float StaminaRegenRate = 15.f; // 초당 회복량
	UPROPERTY(EditAnywhere, Category = "Stats")
	float StaminaConsumptionRate = 20.f; // 초당 소모량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxStamina = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentStamina;
	UFUNCTION()
	void OnRep_CurrentStamina();
	
	/** --- 시야 쿨타임 --- */
	// TODO : 시야 쿨타임 구현 (SCP-096 시야 체크와 연동) 
	
private:
	UPROPERTY()
	TObjectPtr<class ACCDCharacter> OwnerCharacter;
};
