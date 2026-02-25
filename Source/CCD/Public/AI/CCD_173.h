// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCD_173.generated.h"

UCLASS()
class CCD_API ACCD_173 : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCD_173();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// 1. 블루프린트에서 편집할 수 있는 감시 소켓 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
	TArray<FName> ObservationSocketNames;
	
	// 2. 얼마나 정면으로 봐야 멈출 것인가 (0.7 = 약 45도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
	float VisibilityThreshold = 0.7f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// 3. 외부(AI 컨트롤러 등)에서 호출할 시야 확인 함수
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsObserved();

};
