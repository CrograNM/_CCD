
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CCDGameState.generated.h"

UCLASS()
class CCD_API ACCDGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 청소 완료 상태 (복제)
	UPROPERTY(ReplicatedUsing = OnRep_CleaningFinished, BlueprintReadOnly, Category = "GameRule")
	bool bIsCleaningFinished = false;

	UFUNCTION()
	void OnRep_CleaningFinished();
	
	// 전체 공유 목숨
	UPROPERTY(ReplicatedUsing = OnRep_SharedLives, BlueprintReadOnly, Category = "GameRule")
	int32 SharedLives = 5; // 초기값 설정

	UFUNCTION()
	void OnRep_SharedLives();
};
