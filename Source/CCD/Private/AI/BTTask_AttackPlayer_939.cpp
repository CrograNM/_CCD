// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer_939.h"
#include "AIController.h"
#include "AI/CCD_939.h"
#include "Animation/AnimMontage.h"

UBTTask_AttackPlayer_939::UBTTask_AttackPlayer_939()
{
	NodeName = TEXT("Attack 939");
}

EBTNodeResult::Type UBTTask_AttackPlayer_939::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACCD_939* Controlling939 = Cast<ACCD_939>(OwnerComp.GetAIOwner()->GetPawn());
	
	if (Controlling939 && AttackMontage)
	{
		Controlling939->Multicast_PlayAttackMontage(AttackMontage); 

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}