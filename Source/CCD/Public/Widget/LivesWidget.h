// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LivesWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class CCD_API ULivesWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	// 숫자를 받아 텍스트를 업데이트하는 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateLivesDisplay(int32 CurrentLives);

protected:
	// 블루프린트의 Text 변수와 이름을 일치시켜야 함
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LivesText;
	
	UPROPERTY(EditAnywhere)
	int Lives = 0; // 초기 목숨 수 (디자인에 따라 조정 가능)
};
