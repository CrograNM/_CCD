// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer_173.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTTask_AttackPlayer_173::UBTTask_AttackPlayer_173()
{
	NodeName = TEXT("Attack Player");
}

EBTNodeResult::Type UBTTask_AttackPlayer_173::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	// 블랙보드에서 TargetActor(플레이어)를 가져옴
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (TargetActor)
	{
		// 1. 데미지 입히기
		UGameplayStatics::ApplyDamage(
			TargetActor, 
			DamageAmount, 
			AIC, 
			AIC->GetPawn(), 
			UDamageType::StaticClass()
		);

		// 2. 공격 사운드

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}