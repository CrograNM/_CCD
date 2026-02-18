#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EMopActor.generated.h"

UCLASS()
class CCD_API ACCD_EMopActor : public ACCD_EquipActor_Base
{
	GENERATED_BODY()

public:
	ACCD_EMopActor();
	virtual void ExecuteAction() override;

protected:
	virtual void BeginPlay() override;
};
