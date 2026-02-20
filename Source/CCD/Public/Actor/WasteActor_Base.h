
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
	
	
	/** --- 사운드 관련 --- */
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundBase> HitSound;

	// 소리 발생 최소 충격량
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	float HitSoundThreshold = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	float HitSoundCoolDown = 0.25f;
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundAttenuation> HitAttenuation;

	// 사운드 중첩 방지용
	float LastSoundTime = 0.0f;

	// 충돌 이벤트 함수
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
public:
	
	UFUNCTION()
	void UpdatePhysicsReplicates(bool inReplicates);
};
