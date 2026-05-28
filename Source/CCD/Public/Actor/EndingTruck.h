// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelTransitionBase.h"
#include "EndingTruck.generated.h"

UCLASS()
class CCD_API AEndingTruck : public ALevelTransitionBase
{
	GENERATED_BODY()

public:
	AEndingTruck();

protected:
	virtual void StartLevelTravel() override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Ending", meta = (DisplayName = "On Start Level Travel"))
	void BP_OnStartLevelTravel(int32 TotalPlayers);
};
