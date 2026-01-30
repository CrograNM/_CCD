// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/WasteActor_Base.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWasteActor_Base::AWasteActor_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 복제 설정
	bReplicates = true;
	bNetLoadOnClient = true;
	SetReplicatingMovement(true);
	
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

// Called when the game starts or when spawned
void AWasteActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
	
}

// Called every frame
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
		// [수정] 서버는 물리 핸들을 사용해야 하므로 시뮬레이션을 끄면 안 됩니다.
		// 오직 클라이언트에서만 물리 엔진이 서버 복제 위치와 싸우지 않도록 끕니다.
		if (!HasAuthority())
		{
			MeshComp->SetSimulatePhysics(false);
		}
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // 캐릭터와 충돌 방지
	}
	else
	{
		// 놓았을 때는 다시 물리를 켭니다.
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}