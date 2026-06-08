// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCD_KillZone.generated.h"

class UBoxComponent;

UCLASS()
class CCD_API ACCD_KillZone : public AActor
{
	GENERATED_BODY()

public:
	ACCD_KillZone();

protected:
	virtual void BeginPlay() override;

	// 트리거 영역 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerZone;
	
	// 무언가 영역에 들어왔을 때 호출될 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
};
