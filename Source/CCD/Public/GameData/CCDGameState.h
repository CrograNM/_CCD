
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
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameRule")
	TArray<FString> ReplicatedClearedMapPaths;
	
	/** 게임 오버 여부를 나타내는 복제 변수 */
	UPROPERTY(ReplicatedUsing = OnRep_IsGameOver, BlueprintReadOnly, Category = "GameFlow")
	bool bIsGameOver = false;

	UFUNCTION()
	void OnRep_IsGameOver();
	
protected:
	virtual void BeginPlay() override;
	
	/** 에디터에서 할당할 게임 오버 위젯 클래스 */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;
};
