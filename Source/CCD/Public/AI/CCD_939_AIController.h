// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "CCD_939_AIController.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API ACCD_939_AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ACCD_939_AIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(EditAnywhere, Category = "AI")
	class UBehaviorTree* BTAsset;
	
	const FName TargetActorKey = TEXT("TargetActor");
	const FName LoudLocationKey = TEXT("LoudLocation");
};
