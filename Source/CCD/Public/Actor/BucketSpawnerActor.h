
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	bool IsSpawnAreaClear() const; // 영역 내부에 다른 액터가 있는지 확인하는 함수
	
	UFUNCTION()
	void OnSpawnAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSpawnAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MainMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;
	
	UPROPERTY()
	UMaterialInstanceDynamic* ButtonMaterial; // 시각적 업데이트를 위한 머티리얼 인스턴스
	
	// 스폰 영역 (충돌체)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> SpawnArea;
	
	// --- 소환 설정 ---
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AActor> BucketClass; // 소환할 양동이 클래스

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FName SpawnSocketName = TEXT("BucketSocket"); // 소켓 이름
	
	// --- 제어 변수 ---
	UPROPERTY(ReplicatedUsing=OnRep_CanSpawn) 
	bool bCanSpawn = true;
	
	UFUNCTION()
	void OnRep_CanSpawn();
	
	virtual void ExecuteSpawning(); // 실제로 양동이를 소환하는 함수 (타이머에서 호출, 역재생 로직 포함)
	FTimerHandle SpawnTimerHandle;
	
	// --- 멀티캐스트 함수 ---
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlaySequence(); // 모든 클라이언트에서 시퀀스를 정방향으로 재생

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlayReverseSequence(); // 모든 클라이언트에서 시퀀스를 역방향으로 재생
	
	// --- VFX/SFX ---
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<USoundBase> SpawnSound1; // 정방향 애니메이션 재생 시
	
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<USoundBase> SpawnSound2; // 역방향 애니메이션 재생 시

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraSystem> SpawnEffect; // 니아가라 파티클 시스템
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
