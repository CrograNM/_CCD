
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
	
	// --- 멀티캐스트 함수 ---
	virtual void Multicast_PlaySequence() override; // 모든 클라이언트에서 시퀀스를 정방향으로 재생
	 
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlaySequence2(); // 모든 클라이언트에서 시퀀스를 정방향으로 재생
	
	virtual void ExecuteSpawning() override;
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
