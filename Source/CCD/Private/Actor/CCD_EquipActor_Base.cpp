
#include "Actor/CCD_EquipActor_Base.h"

ACCD_EquipActor_Base::ACCD_EquipActor_Base()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACCD_EquipActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACCD_EquipActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

