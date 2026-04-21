// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LivesWidget.h"

#include "Actor/SharedLivesManager.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void ULivesWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 월드에서 매니저 찾아서 초기 목숨 수로 UI 업데이트
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASharedLivesManager::StaticClass());
	if (ASharedLivesManager* Manager = Cast<ASharedLivesManager>(ManagerActor))
	{	
		Manager->UpdateLivesUI();
	}
}

void ULivesWidget::UpdateLivesDisplay(int32 CurrentLives)
{
	Lives = CurrentLives;
	
	if (LivesText)
	{
		// 숫자를 텍스트로 변환하여 출력 (예: "LIVES: 10")
		LivesText->SetText(FText::AsNumber(Lives));
	}
}
