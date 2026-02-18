#pragma once

#include "CoreMinimal.h"
#include "CCD_EquipActor_Base.h"
#include "CCD_EMopActor.generated.h"

UCLASS()
class CCD_API ACCD_EMopActor : public ACCD_EquipActor_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACCD_EMopActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
