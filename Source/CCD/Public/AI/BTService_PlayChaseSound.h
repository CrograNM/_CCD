// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_PlayChaseSound.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTService_PlayChaseSound : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_PlayChaseSound();

protected:
	// 노드가 활성화될 때 (Enraged 시퀀스 진입 시)
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 노드가 비활성화될 때 (Enraged 시퀀스 탈출 시)
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 에디터에서 할당할 비명 소리 에셋
	UPROPERTY(EditAnywhere, Category = "Sound")
	class USoundBase* ChaseScreamSound;

	// 현재 재생 중인 사운드를 관리
	UPROPERTY()
	class UAudioComponent* CurrentAudio;
};
