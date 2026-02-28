// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetRandomLocation_939.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_GetRandomLocation_939 : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetRandomLocation_939();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey; //

	UPROPERTY(EditAnywhere, Category = "Settings")
	float SearchRadius = 1000.0f;
};
