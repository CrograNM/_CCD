


#include "Actor/CCD_EScannerActor.h"


// Sets default values
ACCD_EScannerActor::ACCD_EScannerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACCD_EScannerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCD_EScannerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

