
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCD_BodyFragment.generated.h"

class ADecal_StainActor_Base;
class USkeletalMeshComponent;
class UProgressComponent;
class UBurnableComponent;

UCLASS()
class CCD_API ACCD_BodyFragment : public AActor
{
	GENERATED_BODY()

public:
	ACCD_BodyFragment();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_SkeletalMesh)
	TObjectPtr<USkeletalMesh> RepSkeletalMesh;

	UFUNCTION()
	void OnRep_SkeletalMesh();
	
	/** 스폰 후 메쉬를 설정하고 물리 충격을 가하는 함수 */
	void InitFragment(USkeletalMesh* InMesh, FVector Impulse);
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProgressComponent> ProgressComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBurnableComponent> BurnableComp;	
	
	// --- Hit, Sound, VFX ---
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	float HitThreshold = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	float HitCoolDown = 0.25f;
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundBase> HitSound;
	
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundAttenuation> HitAttenuation;
	
	UPROPERTY(EditAnywhere, Category = "Design")
	TObjectPtr<class UNiagaraSystem> HitEffect;
	
	// Spawn Decal
	UPROPERTY(EditDefaultsOnly, Category = "Design")
	TSubclassOf<ADecal_StainActor_Base> DecalStainActorClass;
	
	// 사운드 중첩 방지용
	float LastSoundTime = 0.0f;

	// 충돌 이벤트 함수
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
