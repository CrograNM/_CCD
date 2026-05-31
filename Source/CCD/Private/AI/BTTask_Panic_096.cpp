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
	
	
	UBehaviorTreeComponent* BTComp = &OwnerComp;

	FTimerHandle FinishTimer;
	SCP096->GetWorldTimerManager().SetTimer(FinishTimer, [this, BTComp, SCP096]()
	{
		if (SCP096 && BTComp)
		{
			SCP096->SetState(E096State::Enraged);
			this->FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
		}
	}, 10.0f, false);
	
	return EBTNodeResult::InProgress;
}