
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "EyeCooldownWidget.generated.h"

UCLASS()
class CCD_API UEyeCooldownWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_EyeCooldown;
	
	void UpdateCooldown(float CooldownTime, float CooldownDuration)
	{
		if (ProgressBar_EyeCooldown)
		{
			float CooldownRatio = FMath::Clamp(CooldownTime / CooldownDuration, 0.0f, 1.0f);
			ProgressBar_EyeCooldown->SetPercent(1.0 - CooldownRatio);
		}
	}
};
