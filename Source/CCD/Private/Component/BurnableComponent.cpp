// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"

// Sets default values for this component's properties
UBurnableComponent::UBurnableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBurnableComponent::BeginPlay()
{
	Super::BeginPlay();

	// 액터의 ProgressComponent 찾기
	ProgressComp = GetOwner()->FindComponentByClass<UProgressComponent>();
	
}

// Called every frame
void UBurnableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBurnableComponent::Interact_Implementation(AActor* Interactor)
{
	//
	// 캐릭터의 Physics Handle 함수를 호출하는 로직을 작성
	//
	
	UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Interacted!"), *GetOwner()->GetName());
}

void UBurnableComponent::TakeBurnDamage(float DamageAmount)
{
	BurnHealth -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Damaged, Current HP: %f"), *GetOwner()->GetName(), BurnHealth);

	if (BurnHealth <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Burned!"), *GetOwner()->GetName());
		
		// 점수 컴포넌트가 유효하면 점수 증가
		if (ProgressComp)
		{
			ProgressComp->Notify_ProgressOver();
		}
		
		// 소각된 액터 제거
		GetOwner()->Destroy();
	}
}

