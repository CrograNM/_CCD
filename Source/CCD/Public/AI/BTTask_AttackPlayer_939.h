// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackPlayer_939.generated.h"

/**
 * 
 */
class UAnimMontage;

UCLASS()
class CCD_API UBTTask_AttackPlayer_939 : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AttackPlayer_939();
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* AttackMontage;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};