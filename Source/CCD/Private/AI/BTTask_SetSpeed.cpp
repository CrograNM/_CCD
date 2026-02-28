// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SetSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_SetSpeed::UBTTask_SetSpeed() { NodeName = TEXT("Set Speed 096"); }

EBTNodeResult::Type UBTTask_SetSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ACharacter* Character = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = NewSpeed; // 속도 변경
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}