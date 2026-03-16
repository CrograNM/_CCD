
#include "Actor/EntranceActor.h"
#include "Components/BoxComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"


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
	
	// 모든 플레이어 오버랩 확인 로직 
	int32 TotalPlayers = GetWorld()->GetGameState()->PlayerArray.Num();
	int32 OverlappingPlayers = 0;
	TArray<AActor*> OverlappingActors;
	WatingArea->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		// 폰(Character)이고 실제로 컨트롤러가 소유하고 있는지 확인
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (Pawn->IsPlayerControlled())
			{
				OverlappingPlayers++;
			}
		}
	}
	// 접속자 전원이 영역 안에 들어왔는지 반환
	return (OverlappingPlayers >= TotalPlayers && TotalPlayers > 0);
}

void AEntranceActor::OnWaitingAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && IsWaitingAreaFull())
	{
		bCanStart = true;
		OnRep_CanStart();
	}
}

void AEntranceActor::OnWaitingAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority() && !IsWaitingAreaFull())
	{
		bCanStart = false;
		OnRep_CanStart();
	}
}

void AEntranceActor::OnRep_CanStart()
{
	if (StatusLightMaterial)
	{
		FVector Color = bCanStart ? FVector(0.0f, 5.0f, 0.0f) : FVector(5.0f, 0.0f, 0.0f);
		StatusLightMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
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

void AEntranceActor::StartLevelTravel()
{
	if (!HasAuthority() || NextLevelPath.IsEmpty()) return;

	if (UWorld* World = GetWorld())
	{
		FString TravelURL = NextLevelPath + TEXT("?listen");
		World->ServerTravel(TravelURL);
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
	
	// 2초 후 레벨 이동 타이머 시작
	GetWorldTimerManager().SetTimer(TravelTimerHandle, this, &AEntranceActor::StartLevelTravel, 2.0f, false);
	
	// 중복 상호작용 방지
	bCanStart = false;
	OnRep_CanStart();
}

