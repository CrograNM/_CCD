
#pragma once

#include "CoreMinimal.h"
#include "BucketSpawnerActor.h"
#include "BinSpawnerActor.generated.h"

UCLASS()
class CCD_API ABinSpawnerActor : public ABucketSpawnerActor
{
	GENERATED_BODY()

public:
	ABinSpawnerActor();
	
protected:
	virtual void BeginPlay() override;
	
	void PlaySpawnSound();
	
	// --- 멀티캐스트 함수 ---
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySequence1(); // 모든 클라이언트에서 시퀀스를 정방향으로 재생
	 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySequence2(); // 모든 클라이언트에서 시퀀스를 정방향으로 재생
	
	virtual void ExecuteSpawning() override;

	// --- 스폰 방향 정하기 ---
	bool bIsInteractedFromRight = true;

	// 방향별 소켓 이름 설정
	UPROPERTY(EditAnywhere, Category = "Spawning")
	FName RightSpawnSocket = TEXT("BinSocket_Right"); // X 양수

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FName LeftSpawnSocket = TEXT("BinSocket_Left"); // X 음수
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
