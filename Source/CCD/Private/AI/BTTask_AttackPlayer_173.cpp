// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer_173.h"
#include "AIController.h"
#include "CCDCharacter.h"
#include "AI/CCD_173.h"
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
		if (ACCD_173* SCP173 = Cast<ACCD_173>(OwnerComp.GetAIOwner()->GetPawn()))
		{
			SCP173->PlayRandomAttackSound();
		}

		if (ACCDCharacter* Character = Cast<ACCDCharacter>(TargetActor))
		{
			Character->Die();
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}