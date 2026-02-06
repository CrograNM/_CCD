
#include "Component/WashableComponent.h"

#include "Actor/Decal_StainActor_Base.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

UWashableComponent::UWashableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 네트워크 복제 설정
	SetIsReplicatedByDefault(true);
}

void UWashableComponent::BeginPlay()
{
	Super::BeginPlay();

	// 액터의 ProgressComponent 찾기
	ProgressComp = GetOwner()->FindComponentByClass<UProgressComponent>();
}

void UWashableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWashableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWashableComponent, WashHealth);
	DOREPLIFETIME(UWashableComponent, WashableType);
}

void UWashableComponent::TakeWashDamage(float DamageAmount)
{
	// 권한 확인: 서버에서만 실행되도록 보장
	if (!GetOwner()->HasAuthority()) return;
	if (WashHealth <= 0.f) return;
	
	WashHealth = FMath::Clamp(WashHealth - DamageAmount, 0.f, 100.f);
	OnRep_WashHealth();

	if (WashHealth <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Washed!] : %s"), *GetOwner()->GetName());
		
		// 세척된 액터의 머티리얼을 물 머티리얼과 점점 혼합시킨고, 완전히 물로 변하면 Lifespan을 설정하여 제거
		// 완전히 물이 된 상태에서는 Lifespan 동안 투명도가 점점 줄어들도록 구현
		
		if (ADecal_StainActor_Base* StainActor = Cast<ADecal_StainActor_Base>(GetOwner()))
		{
			StainActor->NotifyCleaned();
		}
		
		// 점수 컴포넌트가 유효하면 점수 증가
		if (ProgressComp)
		{
			ProgressComp->Notify_ProgressOver();
		}
	}
}

void UWashableComponent::OnRep_WashHealth()
{
	// 소유자 액터로 캐스팅하여 마스크 값 업데이트 요청
	if (ADecal_StainActor_Base* StainActor = Cast<ADecal_StainActor_Base>(GetOwner()))
	{
		StainActor->UpdateDecalMaterial();
	}
}

void UWashableComponent::SetWashableType(ECCD_WashableType NewType)
{
	if (GetOwner()->HasAuthority())
	{
		WashableType = NewType;
		OnRep_WashableType();
	}
}

void UWashableComponent::OnRep_WashableType()
{
	if (ADecal_StainActor_Base* StainActor = Cast<ADecal_StainActor_Base>(GetOwner()))
	{
		// OnConstruction 로직을 다시 실행하여 머티리얼을 갱신
		StainActor->OnConstruction(StainActor->GetTransform());
		StainActor->UpdateDecalMaterial();
	}
}