// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WasteActor.generated.h"

class UStaticMeshComponent;
class UProgressComponent;
class UBurnableComponent;

UCLASS()
class CCD_API AWasteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWasteActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProgressComponent* ProgressComp;
	
	// 소각 가능 컴포넌트 -> 상호작용(잡기) 및 화상 데미지 처리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBurnableComponent* BurnableComp;	
};
