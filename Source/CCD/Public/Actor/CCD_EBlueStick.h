
#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EBlueStick.generated.h"

UCLASS()
class CCD_API ACCD_EBlueStick : public ACCD_EquipActor_Base
{
	GENERATED_BODY()

public:
	ACCD_EBlueStick();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
