// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_939_AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Player/CCDCharacter.h"

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
	
	if (Stimulus.WasSuccessfullySensed())
	{
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
		{
			if (BB->GetValueAsObject(TargetActorKey) == nullptr)
			{
				BB->SetValueAsVector(LoudLocationKey, Stimulus.StimulusLocation);
				BB->SetValueAsBool(TEXT("IsInvestigating"), true);
			}
		}
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			ACCDCharacter* SensedPlayer = Cast<ACCDCharacter>(Actor);
			if (SensedPlayer && !SensedPlayer->IsDead())
			{
				GetWorldTimerManager().ClearTimer(TargetLostTimerHandle);
				
				if (BB->GetValueAsObject(TargetActorKey) != Actor)
				{
					BB->SetValueAsObject(TargetActorKey, Actor);
				}
				
				BB->ClearValue(LoudLocationKey);
				BB->SetValueAsBool(TEXT("IsInvestigating"), false);
			}
		}
	}
	else 
	{
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			UObject* CurrentTarget = BB->GetValueAsObject(TargetActorKey);
			if (CurrentTarget == Actor)
			{
				if (!GetWorldTimerManager().IsTimerActive(TargetLostTimerHandle))
				{
					TWeakObjectPtr<ACCD_939_AIController> WeakThis(this);
					
					GetWorldTimerManager().SetTimer(
						TargetLostTimerHandle, 
						FTimerDelegate::CreateLambda([WeakThis]()
						{
							if (WeakThis.IsValid())
							{
								WeakThis->ClearTargetActor();
							}
						}),
						2.0f,
						false
					);
				}
			}
		}
	}
}

void ACCD_939_AIController::ClearTargetActor()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->ClearValue(TargetActorKey);
		BB->ClearValue(TEXT("HasScreamed"));
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}