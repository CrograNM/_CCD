#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EMopActor.generated.h"

class USkeletalMeshComponent;

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
	
	
	// 장비의 외형
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> MeshComp;
	
	
	/** --- 오염 데이터 (Replicated) --- */
	UPROPERTY(ReplicatedUsing = OnRep_Pollution)
	float MopPollution_Blood = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Pollution)
	float MopPollution_Excrement = 0.0f;

	UFUNCTION()
	void OnRep_Pollution();

	void UpdateMopMaterial();
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundBase> MopSwingSound;
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundBase> MopWashSound;
	
	UPROPERTY(EditAnywhere, Category = "Design | Effects")
	TObjectPtr<class UNiagaraSystem> MopWashEffect;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayWashEffect(const FVector_NetQuantize& ImpactPoint, const FVector_NetQuantizeNormal& ImpactNormal);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMopSwingSound();
	
	// 오염 걸레질로 인한 핏자국을 생성한 횟수
	UPROPERTY(Replicated)
	int32 SpilledStainCount = 0;
	
private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMopMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Design")
	int32 MaxUseCount = 5;	// 세척 없이 사용 가능한 최대 횟수
};
