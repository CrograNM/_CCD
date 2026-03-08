
#include "Component/CCD_StatComponent.h"

UCCD_StatComponent::UCCD_StatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // (10 FPS)
}

void UCCD_StatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCD_StatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

