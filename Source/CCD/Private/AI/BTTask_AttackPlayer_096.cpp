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
		
		TWeakObjectPtr<UBehaviorTreeComponent> WeakBTComp(&OwnerComp);
		TWeakObjectPtr<ACCD_096> WeakSCP096(SCP096);
		TWeakObjectPtr<UBlackboardComponent> WeakBB(BB);
		TWeakObjectPtr<AAIController> WeakAIC(AIC);
		TWeakObjectPtr<UBTTask_AttackPlayer_096> WeakThis(this);
		
		UAnimMontage* CachedAttackMontage = AttackMontage; 
		
		FTimerHandle KillTimerHandle;
		SCP096->GetWorldTimerManager().SetTimer(KillTimerHandle, [WeakBTComp, WeakSCP096, WeakBB, WeakAIC, WeakThis, CachedAttackMontage]()
		{
			if (WeakBTComp.IsValid() && WeakSCP096.IsValid() && WeakBB.IsValid() && WeakAIC.IsValid() && WeakThis.IsValid())
			{
				UBehaviorTreeComponent* BTComp = WeakBTComp.Get();
				ACCD_096* SCP096 = WeakSCP096.Get();
				UBlackboardComponent* BBComp = WeakBB.Get();
				AAIController* AICont = WeakAIC.Get();
				UBTTask_AttackPlayer_096* MyTask = WeakThis.Get();

				if (CachedAttackMontage)
				{
					SCP096->StopAnimMontage(CachedAttackMontage);
				}
				
				AActor* NextTarget = SCP096->GetNextTarget();
				if (NextTarget)
				{
					BBComp->SetValueAsObject(TEXT("TargetActor"), NextTarget);
					SCP096->SetState(E096State::Enraged); 
					AICont->StopMovement();
				}
				else
				{
					SCP096->SetState(E096State::Idle);
				}
				
				MyTask->FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
			}
		}, 1.5f, false);
		
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}