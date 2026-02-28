// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_GetRandomLocation_939.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_GetRandomLocation_939::UBTTask_GetRandomLocation_939()
{
	NodeName = TEXT("Get Random Location 939");
}

EBTNodeResult::Type UBTTask_GetRandomLocation_939::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	if (ControllingPawn && NavSys)
	{
		FNavLocation NextLocation;
		// 반지름 내의 랜덤 위치 탐색
		if (NavSys->GetRandomReachablePointInRadius(ControllingPawn->GetActorLocation(), SearchRadius, NextLocation))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(PatrolLocationKey.SelectedKeyName, NextLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}