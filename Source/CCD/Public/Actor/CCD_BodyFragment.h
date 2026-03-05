
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCD_BodyFragment.generated.h"

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
	
};
