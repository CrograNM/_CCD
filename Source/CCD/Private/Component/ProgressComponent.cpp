// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ProgressComponent.h"
#include "ProgressManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UProgressComponent::UProgressComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UProgressComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// 월드에서 ProgressManager 액터를 찾음
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AProgressManager::StaticClass());
	ProgressManager = Cast<AProgressManager>(ManagerActor);

	if (ProgressManager)
	{
		// 시작하자마자 매니저의 최대치를 내 점수만큼 올림
		ProgressManager->AddMaxProgress(ProgressValue);
	}
}


// Called every frame
void UProgressComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// 액터가 소각되거나 대걸레질이 완료되었을 때 호출
void UProgressComponent::Notify_ProgressOver()
{
	if (ProgressManager)
	{
		ProgressManager->AddCurrentProgress(ProgressValue);
	}
}

