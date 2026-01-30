
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CCDGameMode.generated.h"

UCLASS()
class CCD_API ACCDGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ACCDGameMode();

	// 청소 완료 시 호출될 함수
	void OnCleaningFinished();

protected:
	virtual void BeginPlay() override;

	// 월드에 배치된 ProgressManager 참조
	UPROPERTY()
	class AProgressManager* ProgressManager;
};
