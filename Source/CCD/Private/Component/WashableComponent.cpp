
#include "Component/WashableComponent.h"

#include "Actor/Decal_StainActor_Base.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

UWashableComponent::UWashableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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
		StainActor->UpdateDecalOpacity(GetWashHealthRatio());
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
		// 1. 블루프린트의 Construction Script를 강제로 다시 돌림 (텍스처 교체 로직이 거기 있다면)
		StainActor->RerunConstructionScripts(); 
        
		// 2. 색상 다시 적용 (Rerun으로 날아갔을 수 있으므로)
		StainActor->OnRep_StainColor();
        
		// 3. 투명도 업데이트
		StainActor->UpdateDecalOpacity(GetWashHealthRatio());
	}
}