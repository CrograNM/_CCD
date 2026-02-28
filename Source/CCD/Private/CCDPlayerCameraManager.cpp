
#include "CCDPlayerCameraManager.h"

ACCDPlayerCameraManager::ACCDPlayerCameraManager()
{
	// Pitch 제한 설정 (도 단위)
	ViewPitchMin = -60.0f; // 아래로 
	ViewPitchMax = 45.0f;  // 위로
	
	// 초기화
	CurrentSaturation = 1.0f;
	TargetSaturation = 1.0f;
}

void ACCDPlayerCameraManager::SetDeathEffect(bool bInEnabled)
{
	TargetSaturation = bInEnabled ? 0.0f : 1.0f;
}

void ACCDPlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	// 부모 클래스의 모디파이어 로직을 먼저 수행합니다.
	Super::ApplyCameraModifiers(DeltaTime, InOutPOV);

	// 현재 채도를 목표 채도까지 부드럽게 이동
	if (!FMath::IsNearlyEqual(CurrentSaturation, TargetSaturation, 0.001f))
	{
		CurrentSaturation = FMath::FInterpTo(CurrentSaturation, TargetSaturation, DeltaTime, InterpSpeed);
	}

	// 채도가 1.0보다 작을 때만 포스트 프로세스를 적용
	if (CurrentSaturation < 0.999f)
	{
		InOutPOV.PostProcessSettings.bOverride_ColorSaturation = true;
		
		// FVector4(R, G, B, A)의 각 채널에 보간된 채도 값을 적용합니다.
		InOutPOV.PostProcessSettings.ColorSaturation = FVector4(CurrentSaturation, CurrentSaturation, CurrentSaturation, 1.0f);

		// 밝기도 채도에 맞춰 함께 어두워지게
		float CurrentGain = FMath::Lerp(1.0f, 0.7f, 1.0f - CurrentSaturation);
		InOutPOV.PostProcessSettings.bOverride_ColorGain = true;
		InOutPOV.PostProcessSettings.ColorGain = FVector4(CurrentGain, CurrentGain, CurrentGain, 1.0f);
	}
}