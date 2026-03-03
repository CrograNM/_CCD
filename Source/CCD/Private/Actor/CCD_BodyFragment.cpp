
#include "Actor/CCD_BodyFragment.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"


ACCD_BodyFragment::ACCD_BodyFragment()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	bNetLoadOnClient = true;
	AActor::SetReplicateMovement(true);
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetIsReplicated(true);
	
	// 물리 및 충돌 설정 (CCD_InteractionComponent가 잡을 수 있도록 함)
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
    
	BurnableComp = CreateDefaultSubobject<UBurnableComponent>(TEXT("BurnableComp"));
	BurnableComp->SetIsReplicated(true);
	
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
	ProgressComp->ProgressValue = 5.0f;
}

void ACCD_BodyFragment::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACCD_BodyFragment::InitFragment(UStaticMesh* InMesh, FVector Impulse)
{
	if (InMesh)
	{
		MeshComp->SetStaticMesh(InMesh);
		MeshComp->AddImpulse(Impulse, NAME_None, true);
	}
}

