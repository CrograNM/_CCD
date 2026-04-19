// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "CCD_173_AIController.generated.h"

/**
 * 
 */

UCLASS()
class CCD_API UCCDPathFollowingComponent : public UCrowdFollowingComponent
{
	GENERATED_BODY()
public:
	UCCDPathFollowingComponent()
	{
		bReachTestIncludesAgentRadius = true;
		bReachTestIncludesGoalRadius = true;
	}
};

UCLASS()
class CCD_API ACCD_173_AIController : public ADetourCrowdAIController
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess(APawn* InPawn) override;
	
	ACCD_173_AIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 에디터에서 사용할 비헤이비어 트리 에셋을 선택
	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BTAsset;
};
