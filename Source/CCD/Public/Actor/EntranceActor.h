
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractInterface.h"
#include "EntranceActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class CCD_API AEntranceActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	AEntranceActor();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	bool IsWaitingAreaFull() const; // 영역 내부에 모든 플레이어가 있는지 확인하는 함수
	
	UFUNCTION()
	void OnWaitingAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnWaitingAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MainMesh;
	
	// 문이 양옆으로 열리는 형태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh2;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StatusLightMesh;
	UPROPERTY()
	UMaterialInstanceDynamic* StatusLightMaterial; 
	
	// 영역 (충돌체)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> WatingArea;
	
	// --- 제어 변수 ---
	UPROPERTY(Replicated)
	bool bIsLoading = false; // 레벨 이동 중복 방지 플래그
	
	UPROPERTY(ReplicatedUsing=OnRep_CanStart) 
	bool bCanStart = false;
	UFUNCTION()
	void OnRep_CanStart();
	
	// --- 멀티캐스트 함수 ---
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlaySequence(); // 모든 클라이언트에서 시퀀스를 정방향으로 재생

	// --- Level Transition ---
	UPROPERTY(EditAnywhere, Category = "Design | Level")
	FString NextLevelPath; // 이동할 맵 경로 (예: /Game/_CCD/Maps/InGame)
	FTimerHandle TravelTimerHandle; // 2초 대기를 위한 타이머
	void StartLevelTravel();
	
	// --- VFX/SFX ---
	UPROPERTY(EditAnywhere, Category = "Design | Sound")
	TObjectPtr<USoundBase> StartSound;
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;	
};
