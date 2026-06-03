// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Panic_096.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_Panic_096 : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Panic_096();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* PanicMontage;
	
private:
	FTimerHandle PanicTimerHandle;
};