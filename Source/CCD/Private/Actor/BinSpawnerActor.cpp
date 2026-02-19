
#include "Actor/BinSpawnerActor.h"

#include "Components/BoxComponent.h"

ABinSpawnerActor::ABinSpawnerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	ButtonMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh2"));
	ButtonMesh2->SetupAttachment(RootComponent);
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

