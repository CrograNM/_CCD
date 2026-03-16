// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SetStateToEnraged.h"

#include "AIController.h"
#include "AI/CCD_096.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_SetStateToEnraged::UBTTask_SetStateToEnraged()
{
	NodeName = TEXT("Set State To Enraged");
}

EBTNodeResult::Type UBTTask_SetStateToEnraged::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	ACCD_096* SCP096 = Cast<ACCD_096>(AIC->GetPawn());
	if (SCP096)
	{
		SCP096->SetState(E096State::Enraged);
        
		UE_LOG(LogTemp, Log, TEXT("096 State set to Enraged via Task"));
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}