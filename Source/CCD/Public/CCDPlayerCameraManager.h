
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
	bool bDeathEffectEnabled = false;
};
