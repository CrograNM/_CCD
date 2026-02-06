
#pragma once

#include "CoreMinimal.h"
#include "Actor/WasteActor_Base.h"
#include "WaterBucketActor.generated.h"

class ADecal_StainActor_Base;

UCLASS()
class CCD_API AWaterBucketActor : public AWasteActor_Base
{
	GENERATED_BODY()

public:
	AWaterBucketActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 대걸레를 씻는 함수 : 물이 너무 더러우면 false 반환
	bool WashMop(float& InBloodAmount, float& InExcrementAmount);
	
protected:
	virtual void BeginPlay() override;
	
	// 물 메시 컴포넌트 (블루프린트에서 설정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WaterMeshComp;
	
	// 스폰할 데칼 스테인 액터 블루프린트 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<ADecal_StainActor_Base> DecalStainActorClass;
	
	UPROPERTY()
	UMaterialInstanceDynamic* WaterMaterial; // 시각적 업데이트를 위한 머티리얼 인스턴스

	// 오염도: 0.0f ~ 1.0f (피, 배설물)
	UPROPERTY(ReplicatedUsing = OnRep_Pollution, VisibleAnywhere, BlueprintReadOnly, Category="Pollution")
	float Pollution_Blood {0.0f};
	
	UPROPERTY(ReplicatedUsing = OnRep_Pollution, VisibleAnywhere, BlueprintReadOnly, Category="Pollution")
	float Pollution_Excrement {0.0f};
	
	UFUNCTION()
	void OnRep_Pollution(); // 오염도 변경시 호출
	
	void UpdateWaterColor(); // 물 색상 업데이트
	
	void SpillWater();
};
