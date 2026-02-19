
#include "Actor/BinSpawnerActor.h"

ABinSpawnerActor::ABinSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh2"));
	ButtonMesh->SetupAttachment(RootComponent);
}

void ABinSpawnerActor::BeginPlay()
{
	Super::BeginPlay();
	
	ButtonMesh2->SetMaterial(0, ButtonMaterial);
}

void ABinSpawnerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

