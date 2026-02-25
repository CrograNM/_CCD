

#include "Component/CCD_DeathComponent.h"


#include "Net/UnrealNetwork.h"


UCCD_DeathComponent::UCCD_DeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); // 컴포넌트 복제 활성화
}

void UCCD_DeathComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UCCD_DeathComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCCD_DeathComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 체력과 사망 상태를 네트워크 복제 등록
	DOREPLIFETIME(UCCD_DeathComponent, Health);
	DOREPLIFETIME(UCCD_DeathComponent, bIsDead);
}

void UCCD_DeathComponent::ProcessDamage(float Damage, AController* Instigator)
{
	if (!GetOwner()->HasAuthority() || bIsDead) return;
	UE_LOG(LogTemp, Warning, TEXT("[Death Comp] ProcessDamage %f"), Damage);
	
	Health = FMath::Clamp(Health - Damage, 0.0f, 100.0f);
	
	if (Health <= 0.0f)
	{
		HandleDeath(Instigator);
	}
}

void UCCD_DeathComponent::HandleDeath(AController* Killer)
{
	if (!GetOwner()->HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[Death Comp] HandleDeath"));
	
	bIsDead = true;
	
	// 서버에서 즉시 이벤트 호출 (서버 소유의 캐릭터/컨트롤러 대응)
	OnDeath.Broadcast(Killer);
}

void UCCD_DeathComponent::OnRep_Health()
{
	// UI 업데이트
}

void UCCD_DeathComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		// 모든 클라이언트에서 사망 이벤트 발생
		OnDeath.Broadcast(nullptr); 
	}
}
