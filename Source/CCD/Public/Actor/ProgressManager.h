// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProgressManager.generated.h"

UCLASS()
class CCD_API AProgressManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProgressManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Progress")
	float CurrentProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Progress")
	float MaxProgress = 0.0f;
	
public:
	// Max Progress를 동적으로 추가 (쓰레기 생성 시 호출)
	void AddMaxProgress(float Value) { MaxProgress += Value; UpdateUI(); }

	// 현재 Progress 기록 (청소 완료 시 호출)
	void AddCurrentProgress(float Value) { CurrentProgress += Value; UpdateUI(); }
	
	// 현재 진행도 갱신 (GameMode에 결과 전달)
	void UpdateProgress();
	
	void UpdateUI();
};
