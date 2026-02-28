// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_PlayTriggeredSound.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UBTService_PlayTriggeredSound : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_PlayTriggeredSound();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	class USoundBase* TriggeredSound;
	
	UPROPERTY()
	class UAudioComponent* CurrentAudio;
};
