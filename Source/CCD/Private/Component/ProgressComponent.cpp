// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ProgressComponent.h"
#include "ProgressManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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
		if (!GetOwner()->HasAuthority()) return;
		ProgressManager->AddMaxProgress(ProgressValue);
	}
}


// Called every frame
void UProgressComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UProgressComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UProgressComponent, ProgressValue);
}

// 서버에서 복제된 ProgressValue가 변경될 때 호출 -> 이거 에디터에서 바꾸면 적용 되는지 모르곘네 (추후 확인하기)
void UProgressComponent::OnRep_ProgressValue()
{
}

// 액터가 소각되거나 대걸레질이 완료되었을 때 호출
void UProgressComponent::Notify_ProgressOver() const
{
	if (!GetOwner()->HasAuthority()) return;
	
	if (ProgressManager)
	{
		ProgressManager->AddCurrentProgress(ProgressValue);
	}
}