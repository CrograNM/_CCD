// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_173_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void ACCD_173_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
	}
}
