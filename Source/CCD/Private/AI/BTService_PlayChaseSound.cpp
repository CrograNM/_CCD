// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_PlayChaseSound.h"
#include "AIController.h"
//#include "GameFramework/Character.h"
#include "AI/CCD_096.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_PlayChaseSound::UBTService_PlayChaseSound()
{
	NodeName = TEXT("Play Chase Sound 096");
	Interval = 0.5f; 
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_PlayChaseSound::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	if (ACCD_096* SCP096 = Cast<ACCD_096>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		SCP096->PlayChaseSound();
	}
}

void UBTService_PlayChaseSound::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}