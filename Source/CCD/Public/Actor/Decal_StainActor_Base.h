
#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "Decal_StainActor_Base.generated.h"

class UProgressComponent;
class UWashableComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS()
class CCD_API ADecal_StainActor_Base : public ADecalActor
{
	GENERATED_BODY()

public:
	ADecal_StainActor_Base();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// 머티리얼 파라미터를 업데이트하는 함수
	void UpdateDecalMaterial() const;
	
	// 외부에서 오염도를 설정하는 함수 (물 쏟을 때 사용)
	void SetPollution(float InBlood, float InExcrement);
	
	// WashableComponent에서 호출할 함수
	void NotifyCleaned();
	
	UPROPERTY(Replicated)
	float Pollution_Blood = 0.f;

	UPROPERTY(Replicated)
	float Pollution_Excrement = 0.f;

protected:
	virtual void BeginPlay() override;

	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProgressComponent* ProgressComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWashableComponent* WashableComp;
	
	// --- 머티리얼 및 상태 ---
	UPROPERTY(EditAnywhere, Category = "Decal")
	UMaterialInterface* BloodDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Decal")
	UMaterialInterface* ExcrementDecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Decal")
	UMaterialInterface* WaterDecalMaterial;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DecalDMI;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_IsCleaned)
	bool bIsCleaned = false;

	UFUNCTION()
	void OnRep_IsCleaned();
	
	// 클라이언트에서 페이드 아웃 효과를 위해 경과 시간을 직접 관리하는 변수
	float FadeTimeAccumulator = 0.0f;
};
