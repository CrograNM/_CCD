
#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EBlueStick.generated.h"

class UPointLightComponent;
class USphereComponent;

UCLASS()
class CCD_API ACCD_EBlueStick : public ACCD_EquipActor_Base
{
	GENERATED_BODY()

public:
	ACCD_EBlueStick();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ExecuteAction() override;
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	
protected:
	virtual void BeginPlay() override;
	// 장비 외형
	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	// 감지 영역
	UPROPERTY(VisibleAnywhere, Category = "Detection")
	TObjectPtr<USphereComponent> DetectionSphere;
	
	// 장비에서 뿜어져 나오는 시각적 빛
	UPROPERTY(VisibleAnywhere, Category = "Detection")
	TObjectPtr<UPointLightComponent> DeviceLight;
	
	UPROPERTY(EditAnywhere, Category = "Detection")
	class UMaterialParameterCollection* UVLightMPC;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsLEDOn)
	bool bIsLEDOn = false;
	UFUNCTION()
	void OnRep_IsLEDOn();
	
	void UpdateUVLightEffect();
	
	UPROPERTY()
	UMaterialInstanceDynamic* MeshLightMID; // 메쉬 전용 비주얼 업데이트 머티리얼 인스턴스
	void UpdateMeshLightMID();
	
private:
	const int32 DetectionStencilValue = 2; // 감지 시 적용할 스텐실 값 (포스트 프로세스에서 2번을 푸른 빛으로 설정 가정)
};
