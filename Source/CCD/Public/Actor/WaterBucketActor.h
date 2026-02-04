
#pragma once

#include "CoreMinimal.h"
#include "Actor/WasteActor_Base.h"
#include "WaterBucketActor.generated.h"

UCLASS()
class CCD_API AWaterBucketActor : public AWasteActor_Base
{
	GENERATED_BODY()

public:
	AWaterBucketActor();
	
	// 대걸레와 상호작용하는 함수 등
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
};
