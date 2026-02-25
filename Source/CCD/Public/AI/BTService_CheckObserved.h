// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckObserved.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTService_CheckObserved : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_CheckObserved();

protected:
	// 서비스가 실행될 때 주기적으로 호출되는 함수
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceToPlayerKey;

	// 플레이어를 타겟으로 지정할 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};