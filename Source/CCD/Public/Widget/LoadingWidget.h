// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class CCD_API ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 로딩바 보간용 변수 */
	float TargetProgress = 0.0f;
	float CurrentDisplayProgress = 0.0f;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PercentText;
};
