// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer_096.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CCD_096.h"
#include "Player/CCDCharacter.h"
#include "Animation/AnimMontage.h"

UBTTask_AttackPlayer_096::UBTTask_AttackPlayer_096()
{
	NodeName = TEXT("Attack and Reset 096");
	AttackMontage = nullptr;
}

EBTNodeResult::Type UBTTask_AttackPlayer_096::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    
	ACCDCharacter* TargetPlayer = Cast<ACCDCharacter>(BB->GetValueAsObject(TEXT("TargetActor")));
	ACCD_096* SCP096 = Cast<ACCD_096>(AIC->GetPawn());

	if (TargetPlayer && SCP096 && !TargetPlayer->IsDead())
	{
		TargetPlayer->Die(); 
		SCP096->Multicast_PlayKillSound();
		
		if (AttackMontage)
		{
			SCP096->PlayAnimMontage(AttackMontage);
		}
		
		UBehaviorTreeComponent* BTComp = &OwnerComp;
		
		FTimerHandle KillTimerHandle;
		SCP096->GetWorldTimerManager().SetTimer(KillTimerHandle, [this, BTComp, SCP096, BB, AIC]()
		{
			if (SCP096 && BTComp)
			{
				SCP096->StopAnimMontage(AttackMontage);
				
				AActor* NextTarget = SCP096->GetNextTarget();
				if (NextTarget)
				{
					BB->SetValueAsObject(TEXT("TargetActor"), NextTarget);
					SCP096->SetState(E096State::Enraged); 
					AIC->StopMovement();
				}
				else
				{
					SCP096->SetState(E096State::Idle);
				}
				
				this->FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
			}
		}, 1.5f, false);
		
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}