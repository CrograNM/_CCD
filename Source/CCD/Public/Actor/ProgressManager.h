
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProgressManager.generated.h"

UCLASS()
class CCD_API AProgressManager : public AActor
{
	GENERATED_BODY()

public:
	AProgressManager();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;	
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_Progress, VisibleAnywhere, Category = "Progress")
	float CurrentProgress = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Progress, VisibleAnywhere, Category = "Progress")
	float MaxProgress = 0.0f;

	UFUNCTION()
	void OnRep_Progress();
	
public:
	// 진행도 비율 반환
	UFUNCTION(BlueprintCallable, Category = "Progress")
	float GetProgressRatio() const { return (MaxProgress > 0.0f) ? (CurrentProgress / MaxProgress) : 0.0f; }
	
	// Max Progress를 동적으로 추가 (쓰레기 생성 시 호출)
	void AddMaxProgress(float Value);

	// 현재 Progress 기록 (청소 완료 시 호출)
	void AddCurrentProgress(float Value);
	
	// 현재 진행도 갱신 (GameMode에 결과 전달)
	void UpdateProgress();
	
	void UpdateUI();
};
