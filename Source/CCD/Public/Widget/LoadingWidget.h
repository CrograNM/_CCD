// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

class UTextBlock;
class UProgressBar;
class USoundBase;
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading|Sound")
	TObjectPtr<USoundBase> FinishSound;
	
	bool bHasPlayedFinishSound = false;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PercentText;
};
