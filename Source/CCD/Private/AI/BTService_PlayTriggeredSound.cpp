// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_PlayTriggeredSound.h"
#include "AIController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UBTService_PlayTriggeredSound::UBTService_PlayTriggeredSound()
{
	NodeName = TEXT("Play Triggered Sound 096");
    
	// 서비스의 반응 속도 설정
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_PlayTriggeredSound::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UE_LOG(LogTemp, Log, TEXT("096 Panic Sound Started!"));

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    
	// 소리 에셋이 있고 조종 중인 캐릭터가 있다면 재생 시작
	if (ControllingPawn && TriggeredSound)
	{
		// 소리를 캐릭터에 부착하여 생성
		CurrentAudio = UGameplayStatics::SpawnSoundAttached(
			TriggeredSound, 
			ControllingPawn->GetRootComponent(), 
			NAME_None,
			FVector::ZeroVector, 
			EAttachLocation::KeepRelativeOffset, 
			false
		);
	}
}

void UBTService_PlayTriggeredSound::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CurrentAudio && CurrentAudio->IsPlaying())
	{
		CurrentAudio->Stop();
		UE_LOG(LogTemp, Log, TEXT("096 Triggered Sound Stopped"));
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}