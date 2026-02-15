
#include "Actor/BucketSpawnerActor.h"
#include "Components/BoxComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

ABucketSpawnerActor::ABucketSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	// 메쉬 컴포넌트 설정
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = MainMesh;
	
	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	ButtonMesh->SetupAttachment(RootComponent);
	
	// 스폰 영역 설정
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ABucketSpawnerActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (ButtonMesh)
	{
		ButtonMaterial = ButtonMesh->CreateAndSetMaterialInstanceDynamic(0);
		OnRep_CanSpawn(); // 초기 상태에 맞게 버튼 색상 설정
	}
	
	if (HasAuthority())
	{
		SpawnArea->OnComponentBeginOverlap.AddDynamic(this, &ABucketSpawnerActor::OnSpawnAreaBeginOverlap);
		SpawnArea->OnComponentEndOverlap.AddDynamic(this, &ABucketSpawnerActor::OnSpawnAreaEndOverlap);
	}
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

void ABucketSpawnerActor::OnSpawnAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsSpawnAreaClear())
	{
		bCanSpawn = false;
		OnRep_CanSpawn();
	}
}

void ABucketSpawnerActor::OnSpawnAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsSpawnAreaClear())
	{
		bCanSpawn = true;
		OnRep_CanSpawn();
	}
}

void ABucketSpawnerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABucketSpawnerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABucketSpawnerActor, bCanSpawn);
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
	
	// 서버가 모든 클라이언트에게 애니메이션 재생 명령
	Multicast_PlaySequence();

	// 서버에서 스폰 타이머 시작 (역재생 로직 포함)
	bCanSpawn = false;
	OnRep_CanSpawn();
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABucketSpawnerActor::ExecuteSpawning, 0.5f, false);
}

void ABucketSpawnerActor::OnRep_CanSpawn()
{
	if (bCanSpawn)
	{
		// 스폰 가능 상태로 변경될 때 버튼 색상 변경
		if (ButtonMaterial)
		{
			ButtonMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FVector(0.0f, 5.0f, 0.0f)); // 초록색으로 변경
		}
	}
	else
	{
		// 스폰 불가능 상태로 변경될 때 버튼 색상 변경
		if (ButtonMaterial)
		{
			ButtonMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FVector(5.0f, 0.0f, 0.0f)); // 빨간색으로 변경
		}
	}
}

void ABucketSpawnerActor::ExecuteSpawning()
{
	if (!HasAuthority()) return;
	
	// 양동이 생성
	if (BucketClass)
	{
		FVector SpawnLocation = MainMesh->GetSocketLocation(SpawnSocketName);
		FRotator SpawnRotation = MainMesh->GetSocketRotation(SpawnSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(BucketClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	// 모든 클라이언트에게 역재생 명령
	Multicast_PlayReverseSequence();
}

void ABucketSpawnerActor::Multicast_PlaySequence_Implementation()
{
	// 사운드 재생: 소켓 위치에서 재생
	if (SpawnSound1)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound1, GetActorLocation()); 
	}
	
	if (SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpawnEffect, MainMesh->GetSocketLocation(SpawnSocketName));
	}
	
	// 시퀀스 재생
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
	}
}

void ABucketSpawnerActor::Multicast_PlayReverseSequence_Implementation()
{
	if (SpawnSound2)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound2, GetActorLocation()); 
	}
	
	// 시퀀스 역재생
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->PlayReverse();
	}
}
