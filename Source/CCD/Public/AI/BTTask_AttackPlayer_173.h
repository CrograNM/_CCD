// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackPlayer_173.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_AttackPlayer_173 : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_AttackPlayer_173();

protected:
	// 태스크가 실행될 때 호출되는 핵심 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 공격 대상(플레이어)을 찾기 위한 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// 공격력 설정 (임시)
	UPROPERTY(EditAnywhere, Category = "Settings")
	float DamageAmount = 50.0f;
};
