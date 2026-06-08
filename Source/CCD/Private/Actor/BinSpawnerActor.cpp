
#include "Actor/BinSpawnerActor.h"

#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

ABinSpawnerActor::ABinSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABinSpawnerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABinSpawnerActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !Interactor || !bCanSpawn) return;

	// 방향 판별
	FVector SpawnerLocation = GetActorLocation();
	FVector InteractorLocation = Interactor->GetActorLocation();
	FVector ToInteractor = (InteractorLocation - SpawnerLocation).GetSafeNormal();

	// 양수면 오른쪽, 음수면 왼쪽
	float DotResult = FVector::DotProduct(GetActorForwardVector(), ToInteractor);
	bIsInteractedFromRight = (DotResult > 0.0f);

	// 판별된 방향에 따라 시퀀스 재생 (태그 활용)
	if (bIsInteractedFromRight)
	{
		Multicast_PlaySequence2(); // 오른쪽용 시퀀스 재생 로직 실행
	}
	else
	{
		Multicast_PlaySequence1(); // 왼쪽용 시퀀스 재생 로직 실행
	}

	bCanSpawn = false;
	OnRep_CanSpawn();
}

void ABinSpawnerActor::ExecuteSpawning()
{
	if (!HasAuthority()) return;
	
	// 양동이 생성
	if (BucketClass)
	{
		// 방향에 따라 사용할 소켓 이름 결정
		FName TargetSocket = bIsInteractedFromRight ? RightSpawnSocket : LeftSpawnSocket;
		
		FVector SpawnLocation = MainMesh->GetSocketLocation(TargetSocket);
		FRotator SpawnRotation = MainMesh->GetSocketRotation(TargetSocket);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AActor* SpawnedBucket = GetWorld()->SpawnActor<AActor>(BucketClass, SpawnLocation, SpawnRotation, SpawnParams);
		
		if (SpawnedBucket)
		{
			UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1.5f, nullptr);
		}
	}
}

void ABinSpawnerActor::Multicast_PlaySequence1_Implementation()
{	
	PlaySpawnSound();
	
	// 시퀀스 재생
	UActorSequenceComponent* SequenceComp = FindComponentByTag<UActorSequenceComponent>(FName("Sequence1"));
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
	}
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABinSpawnerActor::ExecuteSpawning, 1.4f, false);
}

void ABinSpawnerActor::Multicast_PlaySequence2_Implementation()
{
	PlaySpawnSound();
	
	// 시퀀스 재생
	UActorSequenceComponent* SequenceComp = FindComponentByTag<UActorSequenceComponent>(FName("Sequence2"));
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		SequenceComp->GetSequencePlayer()->Play();
	}
	
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ABinSpawnerActor::ExecuteSpawning, 1.4f, false);
}

void ABinSpawnerActor::PlaySpawnSound() 
{
	if (SpawnSound1)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound1, GetActorLocation()); 
	}
	
	if (SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpawnEffect, MainMesh->GetSocketLocation(SpawnSocketName));
	}
	
	// 0.5초 후에 사운드2 재생 (시퀀스 타이밍에 맞춰)
	GetWorldTimerManager().SetTimer( SpawnTimerHandle, this, &ABinSpawnerActor::PlaySpawnSound2, 0.5f, false);
}

void ABinSpawnerActor::PlaySpawnSound2()
{
	if (SpawnSound2)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound2, GetActorLocation()); 
	}
}