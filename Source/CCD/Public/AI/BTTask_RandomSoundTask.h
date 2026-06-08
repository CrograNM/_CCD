// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RandomSoundTask.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTTask_RandomSoundTask : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_RandomSoundTask();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 재생할 사운드 목록
	UPROPERTY(EditAnywhere, Category = "Sound")
	TArray<TObjectPtr<USoundBase>> SoundPool;

	// 사운드가 재생될 확률 (0.0 = 0%, 1.0 = 100%)
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PlayProbability = 0.3f;

	// 사운드가 공면성이 있는 3D 입체 음향인지 여부
	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bPlayAtActorLocation = true;
};
