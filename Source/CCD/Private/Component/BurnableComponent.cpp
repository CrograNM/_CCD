

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

UBurnableComponent::UBurnableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 네트워크 복제 설정
	SetIsReplicatedByDefault(true);
}

void UBurnableComponent::BeginPlay()
{
	Super::BeginPlay();

	// 액터의 ProgressComponent 찾기
	ProgressComp = GetOwner()->FindComponentByClass<UProgressComponent>();
	
}

void UBurnableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBurnableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UBurnableComponent, BurnHealth);
}

void UBurnableComponent::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Interacted!"), *GetOwner()->GetName());
}

void UBurnableComponent::TakeBurnDamage(float DamageAmount)
{
	// 권한 확인: 서버에서만 실행되도록 보장
	if (!GetOwner()->HasAuthority()) return;
	
	BurnHealth -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("컴포넌트: %s Damaged, Current HP: %f"), *GetOwner()->GetName(), BurnHealth);
	OnRep_BurnHealth(); // 폭파 효과 등 클라이언트에서 처리할 작업
	
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

void UBurnableComponent::OnRep_BurnHealth()
{
	// 서버가 BurnHealth를 바꾸면 클라이언트의 이 함수가 실행됩니다.
	/*if (BurnHealth < 50.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("클라이언트: 물체가 절반 이상 탔습니다!"));
	}*/
}