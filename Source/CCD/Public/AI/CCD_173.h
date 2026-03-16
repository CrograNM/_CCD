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
	
	// 블루프린트에서 편집할 수 있는 감시 소켓 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
	TArray<FName> ObservationSocketNames;
	
	// 얼마나 정면으로 봐야 멈출 것인가 (0.7 = 약 45도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
	float VisibilityThreshold = 0.7f;

	// 사운드 재생용 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UAudioComponent* ScreamAudio;
	
	// 공격 사운드들을 담을 배열
	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<class USoundBase*> AttackSounds;
	
	// 이동 소리 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UAudioComponent* MoveAudio;
	
	// 이동 사운드 에셋 할당
	UPROPERTY(EditAnywhere, Category = "Settings")
	class USoundBase* MoveSound;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// 외부(AI 컨트롤러 등)에서 호출할 시야 확인 함수
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsObserved();
	
	void PlayRandomAttackSound();

	void StartMoveSound();
	void StopMoveSound();
	void SetMovementInstant(bool bInstant);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackSound(int32 SoundIndex);
	
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartMoveSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopMoveSound();
};
