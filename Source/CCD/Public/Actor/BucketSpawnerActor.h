
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractInterface.h"
#include "BucketSpawnerActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class CCD_API ABucketSpawnerActor : public AActor,  public IInteractInterface
{
	GENERATED_BODY()

public:
	ABucketSpawnerActor();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MainMesh;
	
	// 스폰 영역 (충돌체)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnArea;
	
	// --- 제어 변수 ---
	//UPROPERTY(Replicated) 
	bool bCanSpawn = true;
	
	void SpawnBucket();
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
