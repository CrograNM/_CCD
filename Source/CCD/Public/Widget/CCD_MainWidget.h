
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CCD_MainWidget.generated.h"

class UStaminaWidget;
class UEyeWidget;

UCLASS()
class CCD_API UCCD_MainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStaminaWidget> WBP_Stamina;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEyeWidget> WBP_Eye;
};
