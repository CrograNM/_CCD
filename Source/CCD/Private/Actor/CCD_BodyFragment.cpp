
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
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetIsReplicated(true);
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

void ACCD_BodyFragment::InitFragment(USkeletalMesh* InMesh, FVector Impulse)
{
	if (InMesh)
	{
		MeshComp->SetSkeletalMesh(InMesh);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->AddImpulse(Impulse, NAME_None, true);
	}
}

