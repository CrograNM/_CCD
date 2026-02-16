
#include "Component/CCD_InteractionComponent.h"

UCCD_InteractionComponent::UCCD_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCCD_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCD_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

