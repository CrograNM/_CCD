// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/WashableComponent.h"

#include "Actor/Decal_StainActor_Base.h"
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
	
	WashHealth = FMath::Clamp(WashHealth - DamageAmount, 0.f, 100.f);
    
	// 서버 측 비주얼 업데이트
	OnRep_WashHealth();

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
	// 소유자 액터로 캐스팅하여 마스크 값 업데이트 요청
	if (ADecal_StainActor_Base* StainActor = Cast<ADecal_StainActor_Base>(GetOwner()))
	{
		StainActor->UpdateDecalOpacity(getWashHealthRatio());
	}
}

