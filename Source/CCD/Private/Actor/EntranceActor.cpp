
#include "Actor/EntranceActor.h"
#include "Components/BoxComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AEntranceActor::AEntranceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 메쉬 컴포넌트 설정
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = MainMesh;
	
	DoorMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh1"));
	DoorMesh1->SetupAttachment(RootComponent);
	DoorMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh2"));
	DoorMesh2->SetupAttachment(RootComponent);
	StatusLightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatusLightMesh"));
	StatusLightMesh->SetupAttachment(RootComponent);
	
	// 영역 설정
	WatingArea = CreateDefaultSubobject<UBoxComponent>(TEXT("WaitingArea"));
	WatingArea->SetupAttachment(RootComponent);
	WatingArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AEntranceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEntranceActor, bCanStart);
}

void AEntranceActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (StatusLightMesh)
	{
		StatusLightMaterial = StatusLightMesh->CreateAndSetMaterialInstanceDynamic(0);
		OnRep_CanStart(); // 초기 상태에 맞게 라이트 색상 설정
	}
	
	if (HasAuthority())
	{
		WatingArea->OnComponentBeginOverlap.AddDynamic(this, &AEntranceActor::OnWaitingAreaBeginOverlap);
		WatingArea->OnComponentEndOverlap.AddDynamic(this, &AEntranceActor::OnWaitingAreaEndOverlap);
	}
}

bool AEntranceActor::IsWaitingAreaFull() const
{
	if (!WatingArea) return false;
	
	// TODO: 모든 플레이어 확인 로직 
	
	// 임시 로직 : 한 액터라도 있으면 가득 찬 것으로 간주
	TArray<AActor*> OverlappingActors;
	WatingArea->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor != this)
		{
			return true;
		}
	}
	return false; 
}

void AEntranceActor::OnWaitingAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsWaitingAreaFull())
	{
		bCanStart = true;
		OnRep_CanStart();
	}
}

void AEntranceActor::OnWaitingAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsWaitingAreaFull())
	{
		bCanStart = false;
		OnRep_CanStart();
	}
}

void AEntranceActor::OnRep_CanStart()
{
	if (bCanStart)
	{
		if (StatusLightMaterial)
		{
			StatusLightMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FVector(0.0f, 5.0f, 0.0f)); // 초록색으로 변경
		}
	}
	else
	{
		if (StatusLightMaterial)
		{
			StatusLightMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FVector(5.0f, 0.0f, 0.0f)); // 빨간색으로 변경
		}
	}
}

void AEntranceActor::Multicast_PlaySequence_Implementation()
{
	if (StartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StartSound, GetActorLocation());
	}
	
	// 시퀀스 재생
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
	}
}

void AEntranceActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[Interact] Entrance"));
	
	if (!bCanStart || !IsWaitingAreaFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Entrance] Wait for All Players"));
		return;
	}
	
	// 서버가 모든 클라이언트에게 애니메이션 재생 명령
	Multicast_PlaySequence();
}

