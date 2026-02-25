
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
	void StartDeath();
	
protected:
	virtual void BeginPlay() override;

	// UI(HUD) 인스턴스를 저장할 변수
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidgetInstance;
	
	
	/** --- 사망 연출 관련 --- */
	float PostProcessAlpha = 0.0f;
	FTimerHandle PostProcessTimer;
	
	void UpdateDeathVisuals();

	/** --- 관전 모드 --- */
	UPROPERTY()
	TObjectPtr<AActor> CurrentSpectateTarget;

	void SpectateNextPlayer(); // 키 입력(예: 마우스 클릭) 시 호출
	
	UFUNCTION()
	void HandleTargetDeath(AController* Killer); // 추적 중인 대상이 죽었을 때 호출
};
