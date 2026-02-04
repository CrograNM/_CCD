
#include "Actor/WaterBucketActor.h"

#include "Component/ProgressComponent.h"


AWaterBucketActor::AWaterBucketActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 물 양동이 액터는 초기 진행도를 0으로 설정
	ProgressComp->ProgressValue = 0.0f;
}

void AWaterBucketActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWaterBucketActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

