// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_PlayChaseSound.h"
#include "AIController.h"
//#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_PlayChaseSound::UBTService_PlayChaseSound()
{
	NodeName = TEXT("Play Chase Sound 096");
	Interval = 0.5f; 
}

void UBTService_PlayChaseSound::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (ControllingPawn && ChaseScreamSound)
	{
		CurrentAudio = UGameplayStatics::SpawnSoundAttached(
			ChaseScreamSound, 
			ControllingPawn->GetRootComponent(), 
			NAME_None, 
			FVector::ZeroVector, 
			EAttachLocation::KeepRelativeOffset, 
			false
		);
	}
}

void UBTService_PlayChaseSound::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 격노 상태가 끝나면(플레이어를 잡았거나 놓쳤을 때) 사운드를 즉시 멈춤
	if (CurrentAudio && CurrentAudio->IsPlaying())
	{
		CurrentAudio->Stop();
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}