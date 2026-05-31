// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCD_096.generated.h"

UENUM(BlueprintType)
enum class E096State : uint8
{
	Idle      UMETA(DisplayName = "Idle"),
	Panic     UMETA(DisplayName = "Panic"),
	Enraged   UMETA(DisplayName = "Enraged")
};

UCLASS()
class CCD_API ACCD_096 : public ACharacter
{
	GENERATED_BODY()

public:
	ACCD_096();

	/** --- 서버 전용 상태 설정 함수 --- */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetState(E096State NewState);

	/** --- Getter --- */
	FORCEINLINE E096State GetCurrentState() const { return CurrentState; }
	FORCEINLINE class UBoxComponent* GetFaceTrigger() const { return FaceTrigger; }

	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsTriggered() const;

	void TriggerPanic(AActor* Player);
	void PlayPanicSound();
	void PlayChaseSound();
	void PlayCrySound(USoundBase* InSound);
	void StopScreamSound();
	
	void MarkPlayer(AActor* Player);
	
	bool IsPlayerMarked(AActor* Player) const { return TargetList.Contains(Player); }
	
	AActor* GetNextTarget();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** --- 상태 변수 및 복제 함수 --- */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, Category = "AI")
	E096State CurrentState = E096State::Idle;
	
	// 얼굴을 본 플레이어들의 목록
	UPROPERTY()
	TArray<TObjectPtr<AActor>> TargetList;

	UFUNCTION()
	void OnRep_CurrentState();

	/** --- 컴포넌트 및 세팅 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> FaceTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UAudioComponent> ScreamAudio;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UAudioComponent> ScreamAudio_Secondary;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<class USoundBase> PanicSound;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<class USoundBase> PanicSound_2;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<class USoundBase> ChaseSound;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<class USoundBase> KillSound;
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<class UAnimMontage> PanicMontage;
	

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayKillSound();
	

};
