// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_173_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Navigation/PathFollowingComponent.h"

ACCD_173_AIController::ACCD_173_AIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCCDPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ACCD_173_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (UCCDPathFollowingComponent* PathFollower = Cast<UCCDPathFollowingComponent>(GetPathFollowingComponent()))
	{
		PathFollower->SetAcceptanceRadius(80.f); 
	}

	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
	}
}