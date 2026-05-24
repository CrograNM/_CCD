// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer_173.h"
#include "AIController.h"
#include "Player/CCDCharacter.h"
#include "AI/CCD_173.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTTask_AttackPlayer_173::UBTTask_AttackPlayer_173()
{
	NodeName = TEXT("Attack 173");
}

EBTNodeResult::Type UBTTask_AttackPlayer_173::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	ACCDCharacter* Character = Cast<ACCDCharacter>(TargetActor);
	
	if (!Character || Character->IsDead()) 
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		return EBTNodeResult::Failed;
	}
	
	if (Character->IsInvincible())
	{
		return EBTNodeResult::Failed; 
	}
	
	if (ACCD_173* SCP173 = Cast<ACCD_173>(AIC->GetPawn()))
	{
		SCP173->PlayRandomAttackSound();
	}

	Character->Die();
	
	BB->ClearValue(TargetActorKey.SelectedKeyName);

	return EBTNodeResult::Succeeded;
}