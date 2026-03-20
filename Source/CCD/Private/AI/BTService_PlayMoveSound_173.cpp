// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_PlayMoveSound_173.h"
#include "AIController.h"
#include "Player/CCDCharacter.h"
#include "AI/CCD_173.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_PlayMoveSound_173::UBTService_PlayMoveSound_173()
{
	NodeName = TEXT("Play Move Sound 173");
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_PlayMoveSound_173::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	
	ACCDCharacter* Target = Cast<ACCDCharacter>(BB->GetValueAsObject(FName("TargetActor")));
	if (Target && !Target->IsDead())
	{
		if (ACCD_173* SCP173 = Cast<ACCD_173>(OwnerComp.GetAIOwner()->GetPawn()))
		{
			SCP173->StartMoveSound();
		}
	}
}

void UBTService_PlayMoveSound_173::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ACCD_173* SCP173 = Cast<ACCD_173>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		SCP173->StopMoveSound();
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}