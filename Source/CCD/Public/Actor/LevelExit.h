// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/LevelTransitionBase.h"
#include "LevelExit.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API ALevelExit : public ALevelTransitionBase
{
	GENERATED_BODY()

public:
	ALevelExit();
	
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	virtual bool IsWaitingAreaFull() const override;
};
