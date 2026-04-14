
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractInterface.h"
#include "IncineratorActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class CCD_API AIncineratorActor : public AActor,  public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AIncineratorActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnBurnAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBurnAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MainMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;
	
	// --- 문 제어 변수 ---
	UPROPERTY(ReplicatedUsing = OnRep_DoorOpen) // 모든 플레이어에게 문 상태를 동기화
	bool bIsDoorOpen = true;
	
	UFUNCTION()
	void OnRep_DoorOpen();
	
	// 소각 영역 (충돌체)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> BurnArea;
	
	// 초당 줄 대미지
	UPROPERTY(EditAnywhere, Category = "Design")
	float DamagePerSecond = 20.f;

	// 현재 영역 안에 있는 Burnable 컴포넌트 목록
	UPROPERTY()
	TArray<class UBurnableComponent*> OverlappingBurnables;

	// --- 타이머 관련 ---
	FTimerHandle BurnTimerHandle;

	// 실제로 대미지를 주는 로직을 담은 함수
	void ApplyBurnDamage(float DeltaTime);
	
	// --- VFX/SFX ---
	UPROPERTY(EditAnywhere, Category = "Design | Effects")
	TObjectPtr<USoundBase> DoorSound1; // 정방향 애니메이션 재생 시
	
	UPROPERTY(EditAnywhere, Category = "Design | Effects")
	TObjectPtr<USoundBase> DoorSound2; // 역방향 애니메이션 재생 시
	
	UPROPERTY(EditAnywhere, Category = "Design | Effects")
	TObjectPtr<USoundBase> BurningSound; // 소각로 작동 중 지속적으로 재생되는 사운드
	
	// 지속적인 소각 사운드를 제어할 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> BurningAudioComp;
	
public:
	// 캐릭터가 상호작용(E키) 눌렀을 때 호출됨 -> 소각로 문 열고 닫기
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
