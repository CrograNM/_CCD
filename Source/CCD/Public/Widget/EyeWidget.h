
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "EyeWidget.generated.h"

UCLASS()
class CCD_API UEyeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Eye")
	void CloseEyeAnimation(); // 위젯 애니메이션 실행
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_EyeCooldown;
	
	void UpdateCooldown(float CurrentTime, float CooldownDuration)
	{
		if (ProgressBar_EyeCooldown)
		{
			float CooldownRatio = FMath::Clamp(CurrentTime / CooldownDuration, 0.0f, 1.0f);
			ProgressBar_EyeCooldown->SetPercent(CooldownRatio);
			
			if (CooldownRatio <= 0.0f)
			{
				CloseEyeAnimation();
			}
		}
	}
};
