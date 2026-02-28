// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_096_AIController.h"

#include "AI/CCD_096_States.h"
#include "BehaviorTree/BlackboardComponent.h"

void ACCD_096_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
        
		// 초기 상태를 Idle로 설정
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsEnum(TEXT("AIState"), (uint8)E096State::Idle);
		}
	}
}
