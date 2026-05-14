// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_PlayCrySound.h"
#include "AIController.h"
#include "AI/CCD_096.h"
#include "Components/AudioComponent.h"

UBTService_PlayCrySound::UBTService_PlayCrySound()
{
	NodeName = TEXT("Play Cry Sound 096");
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_PlayCrySound::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (ACCD_096* SCP096 = Cast<ACCD_096>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		SCP096->PlayCrySound(CrySound);
	}
}

void UBTService_PlayCrySound::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	if (ACCD_096* SCP096 = Cast<ACCD_096>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		if (SCP096->GetCurrentState() == E096State::Idle)
		{
			SCP096->StopScreamSound();
		}
	}
}

