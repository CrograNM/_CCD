// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_173_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void ACCD_173_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 1. 비헤이비어 트리 실행
	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
	}

	// 2. 블랙보드 초기값 설정
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			// "TargetActor"라는 이름의 블랙보드 키에 플레이어 할당
			BB->SetValueAsObject(FName("TargetActor"), PlayerPawn);
		}
	}
}
