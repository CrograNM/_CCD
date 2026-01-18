// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

#include "CCDCharacter.generated.h"

UENUM(BlueprintType)
enum class ECCD_EquipmentState : uint8
{
	EES_Hands   UMETA(DisplayName = "Hands"),   // 맨손 (Physics Handle)
	EES_Scanner UMETA(DisplayName = "Scanner"), // 탐지장치
	EES_Mop     UMETA(DisplayName = "Mop")     // 대걸레
};

UCLASS()
class CCD_API ACCDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCDCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected: 
	// - 카메라
	// 현재 1인칭인지 확인하는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bIsFirstPerson = false;
	
	// 1인칭 카메라 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCamera;
	
	// 3인칭 - 카메라를 캐릭터 뒤에 고정시켜줄 지지대
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;
	
	// 3인칭 - 실제 플레이어가 보게 될 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;
	
	// 대걸레 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UStaticMeshComponent> MopMesh;

	// 탐지 장치 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UStaticMeshComponent> ScannerMesh;

	// - 물리 핸들
	// 물건을 집기 위한 핸들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	class UPhysicsHandleComponent* PhysicsHandle;
	
	// - 상호작용
	// 상호작용 실행 함수
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PerformInteract();

	// 상호작용 사거리
	UPROPERTY(EditAnywhere, Category = "Design")
	float InteractRange = 300.f;
	
	// - 장비 상태 관리
	// 1. 현재 장비 상태 변수 (RepNotify 설정)
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentState, BlueprintReadOnly, Category = "Equipment")
	ECCD_EquipmentState EquipmentState = ECCD_EquipmentState::EES_Hands;

	// 2. 상태 변화 시 실행될 함수 (클라이언트용)
	UFUNCTION()
	void OnRep_EquipmentState(ECCD_EquipmentState PreviousState);

	// 3. 서버에서 상태를 변경하기 위한 RPC
	UFUNCTION(Server, Reliable)
	void Server_SetEquipmentState(ECCD_EquipmentState NewState);

	// 상태별 동작 제어 함수
	void HandleEquipmentEffects(ECCD_EquipmentState NewState);
	
	
public:
	// BlueprintCallable: 블루프린트에서 이 함수를 호출할 수 있게 합니다.
	// Category: 블루프린트 우클릭 메뉴에서 어떤 카테고리에 보일지 정합니다.
	
	// 시점 전환 함수
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleView();
	
	// 입력 바인딩용 함수 (예: 숫자키 1, 2, 3)
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToHands() { Server_SetEquipmentState(ECCD_EquipmentState::EES_Hands); }
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToScanner() { Server_SetEquipmentState(ECCD_EquipmentState::EES_Scanner); }
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToMop() { Server_SetEquipmentState(ECCD_EquipmentState::EES_Mop); }
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void TestCurrentState();
};
