// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CCDPlayerController.generated.h"

UCLASS()
class CCD_API ACCDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACCDPlayerController();
	
protected:
	virtual void BeginPlay() override;

	// UI(HUD) 인스턴스를 저장할 변수
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidgetInstance;
};
