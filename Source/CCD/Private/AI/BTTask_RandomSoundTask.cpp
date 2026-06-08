// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_RandomSoundTask.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"

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
			if (bPlayAtActorLocation)
			{
				UGameplayStatics::PlaySoundAtLocation(
					ControlledPawn, 
					SelectedSound, 
					ControlledPawn->GetActorLocation()
				);
			}
			else
			{
				UGameplayStatics::PlaySound2D(ControlledPawn, SelectedSound);
			}
		}
	}

	return EBTNodeResult::Succeeded;
}