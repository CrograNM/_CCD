// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "Decal_StainActor_Base.generated.h"

class UProgressComponent;
class UWashableComponent;

UCLASS()
class CCD_API ADecal_StainActor_Base : public ADecalActor
{
	GENERATED_BODY()

public:
	ADecal_StainActor_Base();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// Decal Component는 부모 클래스에 이미 포함되어 있음
	
	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProgressComponent* ProgressComp;
	
	// 세척 가능 컴포넌트 -> 상호작용(세척) 처리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWashableComponent* WashableComp;

};
