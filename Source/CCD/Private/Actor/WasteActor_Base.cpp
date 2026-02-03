
#include "Actor/WasteActor_Base.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

AWasteActor_Base::AWasteActor_Base()
{
	PrimaryActorTick.bCanEverTick = true;

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

void AWasteActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWasteActor_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWasteActor_Base, bIsGrabbed);
}

void AWasteActor_Base::SetGrabbed(bool bInGrabbed)
{
	if (!HasAuthority()) return;
	bIsGrabbed = bInGrabbed;
	OnRep_IsGrabbed(); // 서버에서도 시각적 처리를 위해 호출
}

void AWasteActor_Base::OnRep_IsGrabbed()
{
	if (bIsGrabbed)
	{
		if (!HasAuthority())
		{
			UpdatePhysicsReplicates(false);
		}
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // 캐릭터와 충돌 방지
	}
	else
	{
		UpdatePhysicsReplicates(true);
		
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}

void AWasteActor_Base::UpdatePhysicsReplicates(bool inReplicates)
{
	bReplicates = inReplicates;
	//MeshComp->SetSimulatePhysics(inReplicates);
}
