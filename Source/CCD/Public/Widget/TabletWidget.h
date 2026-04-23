
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabletWidget.generated.h"

UCLASS()
class CCD_API UTabletWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Tablet")
	void UpdateTablet();
	
	// 현재 맵 이름 반환 (경로 제외, 확장자 제외)
	UFUNCTION(BlueprintCallable, Category = "GameInfo")
	FString GetCurrentCleanMapName() const;

	// 싱글, 멀티 모드 구분 텍스트 반환
	UFUNCTION(BlueprintCallable, Category = "GameInfo")
	FString GetGameNetModeText() const;
};
