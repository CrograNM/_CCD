
#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CCDPlayerCameraManager.generated.h"

UCLASS()
class CCD_API ACCDPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	ACCDPlayerCameraManager();
	
	// 사망 효과(채도 조절)를 켜고 끄는 함수
	void SetDeathEffect(bool bEnabled);
	
protected:
	virtual void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

private:
	// 보간용 변수
	float CurrentSaturation = 1.0f; // 현재 채도 (1: 컬러, 0: 흑백)
	float TargetSaturation = 1.0f;  // 목표 채도
	
	UPROPERTY(EditAnywhere, Category = "DeathEffect")
	float InterpSpeed = 2.0f;      // 보간 속도 (높을수록 빠르게 변함)
};
