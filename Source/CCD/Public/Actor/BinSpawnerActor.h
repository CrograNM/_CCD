
#pragma once

#include "CoreMinimal.h"
#include "BucketSpawnerActor.h"
#include "BinSpawnerActor.generated.h"

UCLASS()
class CCD_API ABinSpawnerActor : public ABucketSpawnerActor
{
	GENERATED_BODY()

public:
	ABinSpawnerActor();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ButtonMesh2; 
};
