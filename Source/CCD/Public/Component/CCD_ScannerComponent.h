// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "CCD_ScannerComponent.generated.h"

UCLASS()
class CCD_API UCCD_ScannerComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UCCD_ScannerComponent();
	
	void UpdateScanner();
	
protected:
	virtual void BeginPlay() override;

private:
	float GetScanActorDistance() const;
	const float MaxScanDistance = 1000.f; // 탐지 최대 거리

	UPROPERTY(VisibleAnywhere, Category = "Scanner")
	TObjectPtr<class UWidgetComponent> ScannerWidgetComp;

	UPROPERTY()
	TObjectPtr<class UScannerWidget> ScannerWidget;
};
