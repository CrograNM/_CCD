
#include "Component/CCD_ViewComponent.h"

UCCD_ViewComponent::UCCD_ViewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCCD_ViewComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCD_ViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

