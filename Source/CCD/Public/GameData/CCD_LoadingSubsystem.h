// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CCD_LoadingSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UCCD_LoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** 로딩 화면 표시 */
	void ShowLoadingScreen(TSubclassOf<UUserWidget> WidgetClass);

	/** 로딩 화면 제거 */
	void HideLoadingScreen();

	/** 현재 로딩 수치 계산 (0~1) */
	float GetLoadingProgress() const;
	
	UFUNCTION(BlueprintPure, Category = "Utils")
	static FString GetCleanPathFromSoftObject(TSoftObjectPtr<UObject> SoftObject);

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidgetInstance;
};
