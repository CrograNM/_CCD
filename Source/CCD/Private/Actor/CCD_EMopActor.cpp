


#include "Actor/CCD_EMopActor.h"


// Sets default values
ACCD_EMopActor::ACCD_EMopActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACCD_EMopActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCD_EMopActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

