#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "CCD_EquipActor_Base.generated.h"

UCLASS()
class CCD_API ACCD_EquipActor_Base : public AActor
{
	GENERATED_BODY()

public:
	ACCD_EquipActor_Base();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

};
