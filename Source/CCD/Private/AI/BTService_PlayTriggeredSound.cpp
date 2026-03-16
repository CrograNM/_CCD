// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_PlayTriggeredSound.h"
#include "AIController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UBTService_PlayTriggeredSound::UBTService_PlayTriggeredSound()
{
	NodeName = TEXT("Play Triggered Sound 096");
    
	// 서비스의 반응 속도 설정
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_PlayTriggeredSound::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

}

void UBTService_PlayTriggeredSound::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}