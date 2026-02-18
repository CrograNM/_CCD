#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EScannerActor.generated.h"

UCLASS()
class CCD_API ACCD_EScannerActor : public ACCD_EquipActor_Base
{
	GENERATED_BODY()

public:
	ACCD_EScannerActor();
	
	virtual void ExecuteAction() override;
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
