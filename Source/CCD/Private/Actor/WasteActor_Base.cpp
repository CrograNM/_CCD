
#include "Actor/WasteActor_Base.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

AWasteActor_Base::AWasteActor_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 복제 설정
	bReplicates = true;
	bNetLoadOnClient = true;
	AActor::SetReplicateMovement(true);
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetIsReplicated(true);
	
	// 컴포넌트 생성 및 포함
	BurnableComp = CreateDefaultSubobject<UBurnableComponent>(TEXT("BurnableComp"));
	BurnableComp->SetIsReplicated(true);
	
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
}

void AWasteActor_Base::BeginPlay()
{
	Super::BeginPlay();
}

void AWasteActor_Base::UpdatePhysicsReplicates(bool inReplicates)
{
	bReplicates = inReplicates;
	//MeshComp->SetEnableGravity(inReplicates);
	//MeshComp->SetSimulatePhysics(inReplicates);
}
