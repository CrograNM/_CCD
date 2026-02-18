
#include "Component/CCD_ScannerComponent.h"

UCCD_ScannerComponent::UCCD_ScannerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCCD_ScannerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCD_ScannerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

