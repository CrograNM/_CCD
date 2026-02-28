// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SetStateToEnraged.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CCD_096_States.h"


UBTTask_SetStateToEnraged::UBTTask_SetStateToEnraged()
{
	NodeName = TEXT("Set State To Enraged");

	// 블랙보드 키 선택 시 Enum 타입만 필터링
	StateKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SetStateToEnraged, StateKey), StaticEnum<E096State>());
}

EBTNodeResult::Type UBTTask_SetStateToEnraged::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsEnum(StateKey.SelectedKeyName, (uint8)E096State::Enraged);
        
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}