
#include "Actor/CCD_EquipActor_Base.h"

#include "Player/CCDCharacter.h"

ACCD_EquipActor_Base::ACCD_EquipActor_Base()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 멀티플레이어 대응
}

void ACCD_EquipActor_Base::OnRep_AttachmentReplication()
{
	if (HasAuthority())
	{
		Super::OnRep_AttachmentReplication();
		return;
	}
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