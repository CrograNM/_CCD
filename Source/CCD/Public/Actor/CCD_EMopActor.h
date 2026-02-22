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
	
	UFUNCTION(BlueprintCallable, Category = "Mop")
	void PerformMopTrace();

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

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMopMaterial;
};
