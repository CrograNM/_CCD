
#include "Actor/BucketSpawnerActor.h"
#include "Components/BoxComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"

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

void ABucketSpawnerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABucketSpawnerActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[Interact] BucketSpawner Interact Received"));
	
	// 스폰 가능 여부 체크
	if (!bCanSpawn) return;
	// SpawnBucket();
}

void ABucketSpawnerActor::SpawnBucket()
{
	bCanSpawn = false;
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
		
		float SequenceDuration = 4.5f;
	}
}