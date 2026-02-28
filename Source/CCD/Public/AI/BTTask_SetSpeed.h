// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetSpeed.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_SetSpeed : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetSpeed();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float NewSpeed = 600.0f;
};
