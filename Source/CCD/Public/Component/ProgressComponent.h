// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProgressComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UProgressComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProgressComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	// 매니저를 매번 찾지 않도록 저장
	class AProgressManager* ProgressManager;
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int ProgressValue = 10.0f;
	
	// 액터가 소각되거나 대걸레질이 완료되었을 때 호출
	void Notify_ProgressOver();
};
