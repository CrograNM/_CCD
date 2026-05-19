// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedLivesManager.generated.h"

UCLASS()
class CCD_API ASharedLivesManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASharedLivesManager();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateLivesUI(); // UI 갱신 함수 (클라이언트에서 호출)
	
	int32 GetCurrentLives() const { return Lives; }
	
	/** 서버에서 실제 청소 로직을 수행할 RPC */
	UFUNCTION(Server, Reliable)
	void Server_SetLives(int32 NewLives);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/** 해당 레벨의 시작 목숨 수 */
	UPROPERTY(ReplicatedUsing=OnRep_Lives, EditAnywhere, Category = "Design | Rules")
	int32 Lives = 10;

	UFUNCTION()
	void OnRep_Lives();
	
	/** 무한 목숨 여부 (로비 등에서 사용) */
	UPROPERTY(EditAnywhere, Category = "Design | Rules")
	bool bIsInfiniteLives = false;

public:
	/** 서버에서 목숨 차감 요청 */
	bool AttemptDecrementLife();
};
