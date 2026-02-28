// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CCD_096_AIController.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API ACCD_096_AIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	class UBehaviorTree* BTAsset;
};
