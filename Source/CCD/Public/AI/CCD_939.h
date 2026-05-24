// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "CCD_939.generated.h"

UCLASS()
class CCD_API ACCD_939 : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCD_939();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 939의 기본 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float PatrolSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float ChaseSpeed = 600.0f;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// AI 컨트롤러에서 호출할 수 있는 상태 변경 함수
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetMovementState(bool bIsChasing);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* HeadCollision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* BodyCollision;
	
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ExecuteAttack();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(UAnimMontage* MontageToPlay);
};
