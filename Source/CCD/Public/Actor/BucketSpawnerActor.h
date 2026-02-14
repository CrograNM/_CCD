
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BucketMesh; // 애니메이션 용 양동이 메쉬
	
	// 스폰 영역 (충돌체)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnArea;
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
