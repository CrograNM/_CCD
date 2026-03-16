// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackPlayer_096.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_AttackPlayer_096 : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_AttackPlayer_096();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
