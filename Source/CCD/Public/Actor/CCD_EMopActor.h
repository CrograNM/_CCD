#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EMopActor.generated.h"

UCLASS()
class CCD_API ACCD_EMopActor : public ACCD_EquipActor_Base
{
	GENERATED_BODY()

public:
	ACCD_EMopActor();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ExecuteAction() override;

	// 물양동이 등에서 호출할 세척 기능
	bool WashMop(float& OutBlood, float& OutExcrement);

protected:
	virtual void BeginPlay() override;
	
	/** --- 오염 데이터 (Replicated) --- */
	UPROPERTY(ReplicatedUsing = OnRep_Pollution)
	float MopPollution_Blood = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Pollution)
	float MopPollution_Excrement = 0.0f;

	UFUNCTION()
	void OnRep_Pollution();

	void UpdateMopMaterial();
	
private:
	// 내부 트레이스 로직 (기존 캐릭터 로직 이관)
	void PerformMopTrace();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMopMaterial;
};
