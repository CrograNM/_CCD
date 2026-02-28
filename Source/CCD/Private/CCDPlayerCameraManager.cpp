
#include "CCDPlayerCameraManager.h"

ACCDPlayerCameraManager::ACCDPlayerCameraManager()
{
	// Pitch 제한 설정 (도 단위)
	ViewPitchMin = -60.0f; // 아래로 
	ViewPitchMax = 45.0f;  // 위로
}

void ACCDPlayerCameraManager::SetDeathEffect(bool bInEnabled)
{
	bDeathEffectEnabled = bInEnabled;
}

void ACCDPlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	// 부모 클래스의 모디파이어 로직을 먼저 수행합니다.
	Super::ApplyCameraModifiers(DeltaTime, InOutPOV);

	// 사망 효과가 활성화된 경우에만 채도를 0(흑백)으로 변경합니다.
	if (bDeathEffectEnabled)
	{
		// FMinimalViewInfo 구조체 내부의 PostProcessSettings에 접근합니다.
		InOutPOV.PostProcessSettings.bOverride_ColorSaturation = true;
		InOutPOV.PostProcessSettings.ColorSaturation = FVector4(0.0f, 0.0f, 0.0f, 1.0f);

		// 화면을 약간 어둡게 하는 효과 추가 (선택 사항)
		InOutPOV.PostProcessSettings.bOverride_ColorGain = true;
		InOutPOV.PostProcessSettings.ColorGain = FVector4(0.7f, 0.7f, 0.7f, 1.0f);
	}
}