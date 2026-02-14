
#include "Actor/BucketSpawnerActor.h"
#include "Components/BoxComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "TimerManager.h"

ABucketSpawnerActor::ABucketSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	// 메쉬 컴포넌트 설정
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = MainMesh;
	
	// 스폰 영역 설정
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ABucketSpawnerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ABucketSpawnerActor::IsSpawnAreaClear() const
{
	if (!SpawnArea) return false;

	TArray<AActor*> OverlappingActors;
	SpawnArea->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		// 자신(스포너)이 아닌 다른 액터가 하나라도 있으면 비어있지 않은 것으로 판단
		if (Actor && Actor != this)
		{
			return false;
		}
	}

	return true;
}

void ABucketSpawnerActor::CheckAndResetSpawnState()
{
}

void ABucketSpawnerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABucketSpawnerActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[Interact] BucketSpawner Interact Received"));
	
	if (!bCanSpawn || !IsSpawnAreaClear())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BucketSpawner] Bucket Spawn Fail"));
		return;
	}
	SpawnBucket();
}

void ABucketSpawnerActor::SpawnBucket()
{
	bCanSpawn = false;
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		// 1. 애니메이션 정방향 재생
		SequenceComp->GetSequencePlayer()->Play();

		// 2. 0.5초 뒤에 실제 소환 함수 호출
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABucketSpawnerActor::ExecuteSpawning, 0.5f, false);
	}
}
void ABucketSpawnerActor::ExecuteSpawning()
{
	// 1. 양동이 생성
	if (BucketClass)
	{
		FVector SpawnLocation = MainMesh->GetSocketLocation(SpawnSocketName);
		FRotator SpawnRotation = MainMesh->GetSocketRotation(SpawnSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(BucketClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	// 2. 애니메이션 역재생
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->PlayReverse();
	}

	// 3. 다시 0.5초 뒤에 스폰 가능 상태로 변경 (역재생 완료 시점)
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABucketSpawnerActor::ResetSpawnState, 0.5f, false);
}

void ABucketSpawnerActor::ResetSpawnState()
{
	// 역재생 애니메이션이 끝났을 때 호출됨
	// 이제 다시 스폰 '시도'는 가능한 상태로 변경
	bCanSpawn = true;
}
