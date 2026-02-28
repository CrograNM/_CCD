// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_939_AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"

ACCD_939_AIController::ACCD_939_AIController()
{
	// 퍼셉션 컴포넌트 생성
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	
	// 이벤트 바인딩
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACCD_939_AIController::OnPerceptionUpdated);
}

void ACCD_939_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (BTAsset) RunBehaviorTree(BTAsset);
}

void ACCD_939_AIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	// 이미지의 'Successfully Sensed' 분기점
	if (Stimulus.WasSuccessfullySensed())
	{
		// 1. 청각 자극인 경우
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
		{
			// LoudLocation에 소리 발생 지점 저장
			BB->SetValueAsVector(LoudLocationKey, Stimulus.StimulusLocation);
		}
		// 2. 시각 자극인 경우
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			// TargetActor에 감지된 액터 저장
			BB->SetValueAsObject(TargetActorKey, Actor);
		}
	}
	else
	{
		// 감지가 끊겼을 때의 처리 (이미지의 Clear Value 대응)
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			BB->ClearValue(TargetActorKey);
		}
	}
}