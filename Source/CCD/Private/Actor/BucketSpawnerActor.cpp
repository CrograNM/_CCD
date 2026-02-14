
#include "Actor/BucketSpawnerActor.h"

ABucketSpawnerActor::ABucketSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABucketSpawnerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABucketSpawnerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

