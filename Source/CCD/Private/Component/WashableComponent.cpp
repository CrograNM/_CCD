// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/WashableComponent.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UWashableComponent::UWashableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 네트워크 복제 설정
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UWashableComponent::BeginPlay()
{
	Super::BeginPlay();

	// 액터의 ProgressComponent 찾기
	ProgressComp = GetOwner()->FindComponentByClass<UProgressComponent>();
}


// Called every frame
void UWashableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWashableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UWashableComponent, WashHealth);
}

void UWashableComponent::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Interacted!"), *GetOwner()->GetName());
}

void UWashableComponent::TakeWashDamage(float DamageAmount)
{
	// 권한 확인: 서버에서만 실행되도록 보장
	if (!GetOwner()->HasAuthority()) return;
	
	WashHealth -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Damaged, Current HP: %f"), *GetOwner()->GetName(), WashHealth);

	if (WashHealth <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Washed!"), *GetOwner()->GetName());
		
		// 점수 컴포넌트가 유효하면 점수 증가
		if (ProgressComp)
		{
			ProgressComp->Notify_ProgressOver();
		}
		
		// 세척된 액터 제거
		GetOwner()->Destroy();
	}
}

void UWashableComponent::OnRep_WashHealth()
{
	// WashHealth가 변경될 때 클라이언트에서 실행할 로직 작성 (데칼의 투명도 변경 등)
}

