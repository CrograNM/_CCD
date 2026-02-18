
#include "Actor/CCD_EquipActor_Base.h"

#include "CCDCharacter.h"

ACCD_EquipActor_Base::ACCD_EquipActor_Base()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 멀티플레이어 대응

	// 메쉬 생성 및 루트 설정
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 기본적으로 물리 연산, 콜리전 무시
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ACCD_EquipActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
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