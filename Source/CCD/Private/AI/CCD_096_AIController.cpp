// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_096_AIController.h"
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
			/*
			   0: Idle
			   1: Panic
			   2: Enraged
			*/
			BB->SetValueAsEnum(TEXT("AIState"), 0);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ERROR: BTAsset is NULL!"));
		}
	}
}
