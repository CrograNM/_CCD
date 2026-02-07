
#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "Decal_StainActor_Base.generated.h"

class UProgressComponent;
class UWashableComponent;
class UMaterialInstanceDynamic;

UCLASS()
class CCD_API ADecal_StainActor_Base : public ADecalActor
{
	GENERATED_BODY()

public:
	ADecal_StainActor_Base();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// Decal Component는 부모 클래스에 이미 포함되어 있음
	
	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProgressComponent* ProgressComp;
	
	// 세척 가능 컴포넌트 -> 상호작용(세척) 처리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWashableComponent* WashableComp;
	
	// 생성된 다이내믹 머티리얼 인스턴스 저장
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DecalDMI;
	
public:
	// 머티리얼 파라미터를 업데이트하는 함수
	void UpdateDecalOpacity(float NewRatio) const;
};
