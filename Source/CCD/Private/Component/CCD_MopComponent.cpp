
#include "Component/CCD_MopComponent.h"

UCCD_MopComponent::UCCD_MopComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCCD_MopComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCD_MopComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

