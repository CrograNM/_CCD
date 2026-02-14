
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractInterface.h"
#include "BucketSpawnerActor.generated.h"

UCLASS()
class CCD_API ABucketSpawnerActor : public AActor,  public IInteractInterface
{
	GENERATED_BODY()

public:
	ABucketSpawnerActor();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MainMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BucketMesh; // 애니메이션 용 양동이 메쉬
};
