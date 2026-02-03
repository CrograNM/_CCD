
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WasteActor_Base.generated.h"

class UStaticMeshComponent;
class UProgressComponent;
class UBurnableComponent;

UCLASS()
class CCD_API AWasteActor_Base : public AActor
{
	GENERATED_BODY()

public:
	AWasteActor_Base();
	
protected:
	virtual void BeginPlay() override;
	
	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProgressComponent* ProgressComp;
	
	// 소각 가능 컴포넌트 -> 상호작용(잡기) 및 화상 데미지 처리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBurnableComponent* BurnableComp;	
	
public:
	
	UFUNCTION()
	void UpdatePhysicsReplicates(bool inReplicates);
};
