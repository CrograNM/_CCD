
#include "Actor/IncineratorActor.h"
#include "Components/BoxComponent.h"
#include "Component/BurnableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "Net/UnrealNetwork.h"

AIncineratorActor::AIncineratorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	// 메쉬 컴포넌트 설정
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = MainMesh;
	
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent); 
	
	// 소각 영역 설정
	BurnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("BurnArea"));
	BurnArea->SetupAttachment(RootComponent);
	
	// 서버에서만 대미지 판정을 하도록 설정
	BurnArea->SetCollisionProfileName(TEXT("Trigger"));
	
}

void AIncineratorActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 목표 회전값 설정
	StartRotation = DoorMesh->GetRelativeRotation();
	TargetRotation = DoorMesh->GetRelativeRotation();
	
	if (HasAuthority())
	{
		BurnArea->OnComponentBeginOverlap.AddDynamic(this, &AIncineratorActor::OnBurnAreaBeginOverlap);
		BurnArea->OnComponentEndOverlap.AddDynamic(this, &AIncineratorActor::OnBurnAreaEndOverlap);
	}
}

void AIncineratorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority()) return;
	// 소각 영역 내의 Burnable 컴포넌트에 대미지 적용
	for (int32 i = OverlappingBurnables.Num() - 1; i >= 0; --i)
	{
		if (OverlappingBurnables[i] && OverlappingBurnables[i]->GetOwner())
		{
			float Damage = DamagePerSecond * DeltaTime;
			OverlappingBurnables[i]->TakeBurnDamage(Damage);
		}
		else
		{
			OverlappingBurnables.RemoveAt(i);
		}
	}
}

void AIncineratorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AIncineratorActor, bIsDoorOpen);
}

void AIncineratorActor::OnBurnAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (UBurnableComponent* BurnComp = OtherActor->FindComponentByClass<UBurnableComponent>())
		{
			OverlappingBurnables.AddUnique(BurnComp);
		}
	}
}

void AIncineratorActor::OnBurnAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (UBurnableComponent* BurnComp = OtherActor->FindComponentByClass<UBurnableComponent>())
		{
			OverlappingBurnables.Remove(BurnComp);
		}
	}
}

void AIncineratorActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return; // 상태 변경은 서버에서만 수행
	UE_LOG(LogTemp, Warning, TEXT("IncineratorActor::Interact_Implementation"));
	
	// 문 상태 토글
	bIsDoorOpen = !bIsDoorOpen;
    
	// 서버에서도 OnRep 함수를 직접 호출하여 자신의 화면을 갱신합니다.
	OnRep_DoorOpen();
}

void AIncineratorActor::OnRep_DoorOpen()
{
	// 1. 블루프린트에서 추가된 Actor Sequence 컴포넌트를 찾습니다.
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		if (bIsDoorOpen)
		{
			// 문을 여는 방향으로 재생
			SequenceComp->GetSequencePlayer()->Play();
		}
		else
		{
			// 문을 닫는 방향(역재생)으로 재생
			SequenceComp->GetSequencePlayer()->PlayReverse();
		}
	}
}