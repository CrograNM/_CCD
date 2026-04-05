
#include "Actor/CCD_EquipActor_Base.h"

#include "Player/CCDCharacter.h"

ACCD_EquipActor_Base::ACCD_EquipActor_Base()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 멀티플레이어 대응
}

void ACCD_EquipActor_Base::OnRep_AttachmentReplication()
{
	// 로컬 플레이어(Owner)라면 서버의 3인칭 부착 정보를 무시
	if (IsOwnedByLocalPlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipActor] Ignoring attachment replication for local player"));
		return;
	}
	// 다른 플레이어(Proxy)라면 정상적으로 서버의 부착 정보를 따름
	Super::OnRep_AttachmentReplication();
}

void ACCD_EquipActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ACCD_EquipActor_Base::IsOwnedByLocalPlayer() const
{
	if (const ACCDCharacter* CharacterOwner = Cast<ACCDCharacter>(GetOwner()))
	{
		return CharacterOwner->IsLocallyControlled();
	}
	return false;
}

void ACCD_EquipActor_Base::SetEquipmentActive(bool bActive)
{
	bIsActive = bActive;
}

void ACCD_EquipActor_Base::InitializeEquipment(ACCDCharacter* InOwner)
{
	OwnerCharacter = InOwner;
	SetOwner(InOwner); // 네트워크 소유권 설정
}