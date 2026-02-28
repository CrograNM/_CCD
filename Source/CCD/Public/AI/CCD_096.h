// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCD_096.generated.h"

UCLASS()
class CCD_API ACCD_096 : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCD_096();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// 얼굴 감지용 트리거
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* FaceTrigger;

	// 비명 소리 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UAudioComponent* ScreamAudio;

	// 당황 시 재생할 사운드 에셋
	UPROPERTY(EditAnywhere, Category = "Settings")
	class USoundBase* PanicSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TriggerPanic(AActor* Player);
	
	// 플레이어 클래스에서 맞은 컴포넌트가 얼굴인지 확인할 때 사용
	UBoxComponent* GetFaceTrigger() const { return FaceTrigger; }

	// 트리거된 상태인지 확인 (Do Once 로직 구현용)
	bool IsTriggered() const;
};
