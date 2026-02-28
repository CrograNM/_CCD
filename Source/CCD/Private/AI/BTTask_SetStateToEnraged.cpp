// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SetStateToEnraged.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_SetStateToEnraged::UBTTask_SetStateToEnraged()
{
	NodeName = TEXT("Set State To Enraged");
}

EBTNodeResult::Type UBTTask_SetStateToEnraged::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey.SelectedKeyName, 2); 
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}