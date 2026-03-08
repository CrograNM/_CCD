
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EyeAnimWidget.generated.h"

UCLASS()
class CCD_API UEyeAnimWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Eye")
	void CloseEyeAnimation(); // 위젯 애니메이션 실행
};
