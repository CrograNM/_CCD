// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractInterface.h"
#include "LevelTransitionBase.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CCD_API ALevelTransitionBase : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	ALevelTransitionBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// 핵심 로직: 영역 내 모든 플레이어가 있는지 확인
	virtual bool IsWaitingAreaFull() const;

	UFUNCTION()
	void OnWaitingAreaOverlapChange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnWaitingAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// --- 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> WaitingArea;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractVolume;
	
	UPROPERTY(EditAnywhere, Category = "Level Transition|Sound")
	TObjectPtr<USoundBase> DoorOpenSound;

	// --- 제어 변수 ---
	UPROPERTY(ReplicatedUsing = OnRep_CanStart)
	bool bCanStart = false;

	UPROPERTY(Replicated)
	bool bIsLoading = false;
	
	UFUNCTION()
	void OnRep_CanStart();

	// --- 설정값 ---
	UPROPERTY(EditAnywhere, Category = "Level Transition")
	FString NextLevelPath;

	UPROPERTY(EditAnywhere, Category = "Level Transition")
	bool bNeedToCheckProgressOver = false;

	UPROPERTY(EditAnywhere, Category = "Level Transition")
	float TravelDelay = 1.5f; // 상호작용 후 이동까지의 유예 시간
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	TObjectPtr<UStaticMesh> DoorMeshAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition", meta = (MakeEditWidget = true))
	FVector DoorRelativeLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FVector DoorRelativeScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FVector WaitingAreaRelativeScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition", meta = (MakeEditWidget = true))
	FVector WaitingAreaRelativeLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FVector InteractVolumeExtent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition", meta = (MakeEditWidget = true))
	FVector InteractVolumeRelativeLocation;
	
	virtual void StartLevelTravel();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDoorSound();

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
};
