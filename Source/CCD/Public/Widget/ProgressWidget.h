// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "ProgressWidget.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UProgressWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 블루프린트의 ProgressBar 이름을 'ProgressBar_Cleaning'으로 지으면 자동으로 연결됨.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Cleaning;

	// 외부에서 호출할 업데이트 함수
	void UpdatePercent(float NewRatio)
	{
		if (ProgressBar_Cleaning)
		{
			ProgressBar_Cleaning->SetPercent(NewRatio);
		}
	}
};
