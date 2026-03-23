
#include "Actor/CCD_BodyFragment.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

ACCD_BodyFragment::ACCD_BodyFragment()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	bNetLoadOnClient = true;
	AActor::SetReplicateMovement(true);
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetIsReplicated(true);
	
	// 초기에는 물리 시뮬레이션과 충돌 비활성화 (스폰 이후 시점에 활성화하기)
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->SetGenerateOverlapEvents(true);
	
	BurnableComp = CreateDefaultSubobject<UBurnableComponent>(TEXT("BurnableComp"));
	BurnableComp->SetIsReplicated(true);
	
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
	ProgressComp->ProgressValue = 5.0f;
}

void ACCD_BodyFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCD_BodyFragment, RepSkeletalMesh);
}

void ACCD_BodyFragment::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACCD_BodyFragment::InitFragment(USkeletalMesh* InMesh, FVector Impulse)
{
	if (HasAuthority() && InMesh)
	{
		RepSkeletalMesh = InMesh;
		OnRep_SkeletalMesh();
		MeshComp->AddImpulse(Impulse, NAME_None, true);
	}
}

void ACCD_BodyFragment::OnRep_SkeletalMesh()
{
	if (RepSkeletalMesh)
	{
		MeshComp->SetSkeletalMesh(RepSkeletalMesh);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	}
}