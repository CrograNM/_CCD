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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ExecuteAction() override;
	
protected:
	// 서버에서 계산한 거리를 클라이언트에 전달
	UPROPERTY(ReplicatedUsing = OnRep_ScannerDistance)
	float ScannerDistance = -1.0f;

	UFUNCTION()
	void OnRep_ScannerDistance();

	void UpdateScannerUI(); 
	
private:
	float GetScanActorDistance() const;
	const float MaxScanDistance = 1000.f; // 탐지 최대 거리

	UPROPERTY(VisibleAnywhere, Category = "Scanner")
	TObjectPtr<class UWidgetComponent> ScannerWidgetComp;

	UPROPERTY()
	TObjectPtr<class UScannerWidget> ScannerWidget;
};
