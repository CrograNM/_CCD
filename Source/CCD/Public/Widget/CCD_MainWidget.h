
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CCD_MainWidget.generated.h"

class UStaminaWidget;

UCLASS()
class CCD_API UCCD_MainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStaminaWidget> WBP_Stamina;
	
	// 시야, 소음 등 추가 예정
};
