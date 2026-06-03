// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Panic_096.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CCD_096.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

UBTTask_Panic_096::UBTTask_Panic_096()
{
	NodeName = TEXT("Panic Task 096");
	PanicMontage = nullptr;
}

EBTNodeResult::Type UBTTask_Panic_096::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	ACCD_096* SCP096 = Cast<ACCD_096>(AIC->GetPawn());

	if (!SCP096) return EBTNodeResult::Failed;
	
	if (AIC)
	{
		AIC->StopMovement();
	}
	
	TWeakObjectPtr<UBehaviorTreeComponent> WeakBTComp(&OwnerComp);
	TWeakObjectPtr<ACCD_096> WeakSCP096(SCP096);
	TWeakObjectPtr<UBTTask_Panic_096> WeakThis(this);
	
	SCP096->GetWorldTimerManager().ClearTimer(PanicTimerHandle);

	SCP096->GetWorldTimerManager().SetTimer(PanicTimerHandle, [WeakBTComp, WeakSCP096, WeakThis]()
	{
		if (WeakBTComp.IsValid() && WeakSCP096.IsValid() && WeakThis.IsValid())
		{
			UBehaviorTreeComponent* BTComp = WeakBTComp.Get();
			ACCD_096* SCP = WeakSCP096.Get();
			
			if (BTComp->GetActiveNode() == WeakThis.Get())
			{
				SCP->SetState(E096State::Enraged);
				
				BTComp->OnTaskFinished(WeakThis.Get(), EBTNodeResult::Succeeded);
			}
		}
	}, 10.0f, false);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Panic_096::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC)
	{
		if (ACCD_096* SCP096 = Cast<ACCD_096>(AIC->GetPawn()))
		{
			SCP096->GetWorldTimerManager().ClearTimer(PanicTimerHandle);
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}