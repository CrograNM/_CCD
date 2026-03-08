
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCD_StatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float /*CurrentStamina*/, float /*MaxStamina*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEyeCooldownChanged, float /*CooldownTime*/, float /*CooldownDuration*/);
DECLARE_MULTICAST_DELEGATE (FOnEyeClosed);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CCD_API UCCD_StatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCD_StatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** --- Delegate --- */
	FOnStaminaChanged OnStaminaChanged;
	FOnEyeCooldownChanged OnEyeCooldownChanged;
	FOnEyeClosed OnEyeClosed; // Closed = true 일 때 발생
	
	/** --- Getter / Setter --- */
	UFUNCTION(Server, Reliable)
	void Server_SetSpeed(const bool bNewIsRunning);
	void SetIsRunning(const bool bNewIsRunning);
	
	UFUNCTION(Server, Reliable)
	void Server_CloseEye(const bool bNewIsEyeClosed);
	void SetIsEyeClosed(const bool bNewIsEyeClosed);
	
	float GetCurrentStamina() const { return CurrentStamina; }
	float GetMaxStamina() const { return MaxStamina; }
	float GetEyeCooldown() const { return EyeCooldownTime; }
	float GetEyeCooldownDuration() const { return EyeCooldownDuration; }
	bool GetIsObserveActivated() const { return !bIsEyeClosed; }
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 달리기 / 스태미나 --- */
	UPROPERTY(Replicated)
	bool bIsRunning = false;
	
	UPROPERTY(EditAnywhere, Category = "Stats | Speed")
	float RunSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Stats | Speed")
	float WalkSpeed = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Stats | Stamina")
	float StaminaRegenRate = 15.f; // 초당 회복량
	UPROPERTY(EditAnywhere, Category = "Stats | Stamina")
	float StaminaConsumptionRate = 20.f; // 초당 소모량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats | Stamina")
	float MaxStamina = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Stamina")
	float CurrentStamina;
	UFUNCTION()
	void OnRep_CurrentStamina();
	
	/** --- 시야 판정, 쿨타임 --- */
	UPROPERTY(ReplicatedUsing = OnRep_IsEyeClosed)
	bool bIsEyeClosed = false;
	UFUNCTION()
	void OnRep_IsEyeClosed();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEyeClosedAnimation();
	
	UPROPERTY(EditAnywhere, Category = "Stats | Eye")
	float EyeCooldownDuration = 5.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_EyeCooldownTime, VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Eye")
	float EyeCooldownTime = 0.f; // 시야 쿨타임 경과 시간
	UFUNCTION()
	void OnRep_EyeCooldownTime();
	
private:
	UPROPERTY()
	TObjectPtr<class ACCDCharacter> OwnerCharacter;
};
