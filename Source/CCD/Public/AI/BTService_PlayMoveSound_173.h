// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_PlayMoveSound_173.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTService_PlayMoveSound_173 : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_PlayMoveSound_173();
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
