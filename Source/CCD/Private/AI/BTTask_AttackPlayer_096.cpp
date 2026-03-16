// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer_096.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CCD_096.h"
#include "CCDCharacter.h"

UBTTask_AttackPlayer_096::UBTTask_AttackPlayer_096()
{
	NodeName = TEXT("Attack and Reset 096");
}

EBTNodeResult::Type UBTTask_AttackPlayer_096::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    
	ACCDCharacter* TargetPlayer = Cast<ACCDCharacter>(BB->GetValueAsObject(TEXT("TargetActor")));
	ACCD_096* SCP096 = Cast<ACCD_096>(AIC->GetPawn());

	if (TargetPlayer && SCP096 && !TargetPlayer->IsDead())
	{
		SCP096->Multicast_PlayKillSound();
		
		TargetPlayer->Die(); 
		
		SCP096->SetState(E096State::Idle); 

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}