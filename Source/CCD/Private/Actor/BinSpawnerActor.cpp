
#include "Actor/BinSpawnerActor.h"

#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "NiagaraFunctionLibrary.h"
#include "ToolBuilderUtil.h"
#include "Kismet/GameplayStatics.h"

ABinSpawnerActor::ABinSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SpawnSocketName = TEXT("BinSocket");
}

void ABinSpawnerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABinSpawnerActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[Interact] BinSpawner"));
	
	if (!bCanSpawn || !IsSpawnAreaClear())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BinSpawner] Bin Spawn Fail"));
		return;
	}
	
	// 서버가 모든 클라이언트에게 애니메이션 재생 명령
	Multicast_PlaySequence();

	bCanSpawn = false;
	OnRep_CanSpawn();
}

void ABinSpawnerActor::ExecuteSpawning()
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
}

void ABinSpawnerActor::Multicast_PlaySequence()
{
	if (SpawnSound1)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound1, GetActorLocation()); 
	}
	
	if (SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpawnEffect, MainMesh->GetSocketLocation(SpawnSocketName));
	}
	
	// 시퀀스 재생
	UActorSequenceComponent* SequenceComp = FindComponentByTag<UActorSequenceComponent>(FName("Sequence1"));
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
	}
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABinSpawnerActor::Multicast_PlaySequence2, 0.5f, false);
}

void ABinSpawnerActor::Multicast_PlaySequence2_Implementation()
{
	if (SpawnSound2)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound1, GetActorLocation()); 
	}
	
	// 시퀀스 재생
	UActorSequenceComponent* SequenceComp = FindComponentByTag<UActorSequenceComponent>(FName("Sequence2"));
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
	}
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABinSpawnerActor::ExecuteSpawning, 0.5f, false);
}
