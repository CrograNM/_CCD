// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetStateToEnraged.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_SetStateToEnraged : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetStateToEnraged();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StateKey;
};
