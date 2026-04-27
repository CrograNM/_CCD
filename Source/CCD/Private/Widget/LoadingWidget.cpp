// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LoadingWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameData/CCD_LoadingSubsystem.h"
#include "Kismet/GameplayStatics.h" 
#include "Sound/SoundBase.h"

void ULoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (auto* Subsystem = GetGameInstance()->GetSubsystem<UCCD_LoadingSubsystem>())
	{
		// 1. 실제 로딩 퍼센트 가져오기
		TargetProgress = Subsystem->GetLoadingProgress();
	
		// 2. 부드러운 보간 (값이 뚝뚝 끊기지 않게)
		CurrentDisplayProgress = FMath::FInterpTo(CurrentDisplayProgress, TargetProgress, InDeltaTime, 3.0f);
	
		// 3. UI 반영
		if (LoadingProgressBar)
		{
			LoadingProgressBar->SetPercent(CurrentDisplayProgress);
		}
	
		if (PercentText)
		{
			int32 IntPercent = FMath::Clamp(FMath::FloorToInt(CurrentDisplayProgress * 100.0f), 0, 100);
			PercentText->SetText(FText::Format(FText::FromString(TEXT("{0}%")), IntPercent));
		}
		
		if (!bHasPlayedFinishSound && CurrentDisplayProgress >= 0.999f)
		{
			if (FinishSound)
			{
				UGameplayStatics::PlaySound2D(this, FinishSound);
			}
			bHasPlayedFinishSound = true;
		}
	}
}