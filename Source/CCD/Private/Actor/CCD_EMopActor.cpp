
#include "Actor/CCD_EMopActor.h"

ACCD_EMopActor::ACCD_EMopActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACCD_EMopActor::ExecuteAction()
{
	UE_LOG(LogTemp, Warning, TEXT("ExecuteAction :: MOP"));
}

void ACCD_EMopActor::BeginPlay()
{
	Super::BeginPlay();
	
}
