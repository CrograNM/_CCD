// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_RandomSoundTask.h"

#include "AIController.h"
#include "AI/CCD_939.h"

UBTTask_RandomSoundTask::UBTTask_RandomSoundTask()
{
	NodeName = "Play Random State Sound";
}

EBTNodeResult::Type UBTTask_RandomSoundTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();

	if (!ControlledPawn || SoundPool.Num() == 0) return EBTNodeResult::Succeeded;
	
	if (FMath::FRand() <= PlayProbability)
	{
		int32 RandomIndex = FMath::RandRange(0, SoundPool.Num() - 1);
		USoundBase* SelectedSound = SoundPool[RandomIndex];

		if (SelectedSound)
		{
			ACCD_939* SCP939 = Cast<ACCD_939>(ControlledPawn);
			
			if (SCP939)
			{
				SCP939->StateSound = SelectedSound;
				SCP939->Multicast_PlayStateSound(bPlayAtActorLocation);
			}
		}
	}

	return EBTNodeResult::Succeeded;
}