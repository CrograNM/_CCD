
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
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
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
	
	// 현재 누군가에게 잡혀있는지 여부 (복제)
	UPROPERTY(ReplicatedUsing = OnRep_IsGrabbed, BlueprintReadOnly, Category = "State")
	bool bIsGrabbed = false;

	UFUNCTION()
	void OnRep_IsGrabbed();
	
public:
	// 서버에서 잡기 상태를 설정하는 함수
	void SetGrabbed(bool bIsGrabbed);
};
