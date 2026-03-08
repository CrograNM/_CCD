
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "StaminaWidget.generated.h"

UCLASS()
class CCD_API UStaminaWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Stamina;
	
	void UpdateStamina(float CurrentStamina, float MaxStamina)
	{
		if (ProgressBar_Stamina)
		{
			float StaminaRatio = CurrentStamina / MaxStamina;
			ProgressBar_Stamina->SetPercent(StaminaRatio);
		}
	}
};
