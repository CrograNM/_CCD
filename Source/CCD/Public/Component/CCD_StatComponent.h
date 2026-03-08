
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCD_StatComponent.generated.h"

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
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 달리기 / 스태미나 --- */
	UPROPERTY(ReplicatedUsing=OnRep_IsRunning)
	bool bIsRunning = false;
	UFUNCTION()
	void OnRep_IsRunning();
	
	UPROPERTY(EditAnywhere, Category = "Stats")
	float RunSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Stats")
	float WalkSpeed = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Stats")
	float StaminaRegenRate = 15.f; // 초당 회복량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxStamina = 100.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentStamina;
	
	/** --- 시야 쿨타임 --- */
	// TODO : 시야 쿨타임 구현 (SCP-096 시야 체크와 연동) 
	
private:
	UPROPERTY()
	TObjectPtr<class ACCDCharacter> OwnerCharacter;
};
