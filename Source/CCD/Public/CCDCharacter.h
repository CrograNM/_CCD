// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCDCharacter.generated.h"

// --- 전방 선언 ---
class UCameraComponent;
class USpringArmComponent;
class UPhysicsHandleComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class ECCD_EquipmentState : uint8
{
	EES_Hands   UMETA(DisplayName = "Hands"),   // 맨손 (Physics Handle)
	EES_Scanner UMETA(DisplayName = "Scanner"), // 탐지장치
	EES_Mop     UMETA(DisplayName = "Mop")		// 대걸레
};

UCLASS()
class CCD_API ACCDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACCDCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 카메라 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bIsFirstPerson = false;
	
	/** --- 장비 및 메시 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MopMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ScannerMesh;

	UPROPERTY(ReplicatedUsing = OnRep_EquipmentState, BlueprintReadOnly, Category = "Equipment")
	ECCD_EquipmentState EquipmentState = ECCD_EquipmentState::EES_Hands;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	ECCD_EquipmentState PendingEquipmentState = ECCD_EquipmentState::EES_Hands;
	
	/** --- 애니메이션 몽타주 제어 --- */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;	// 장비 교체 몽타주

	bool bIsUnequipping = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsActionInProgress = false;

public:
	/** --- Getter --- */
	FORCEINLINE ECCD_EquipmentState GetEquipmentState() const { return EquipmentState; }

	/** --- 블루프린트 호출 가능 함수 --- */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleView();
	
	UFUNCTION(Server, Reliable)
	void Server_ToggleView(bool bNewIsFirstPerson);		// ToggleView
	void ApplyViewMode(bool bFirstPerson);				// ToggleView
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PerformInteract();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToHands() { Server_SetEquipmentState(ECCD_EquipmentState::EES_Hands); }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToScanner() { Server_SetEquipmentState(ECCD_EquipmentState::EES_Scanner); }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToMop() { Server_SetEquipmentState(ECCD_EquipmentState::EES_Mop); }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void HandleEquipNotify();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation")
	void Server_PlayActionOfState();
	
protected:
	/** --- 네트워크 & 상태 동기화 --- */
	UFUNCTION()
	void OnRep_EquipmentState(ECCD_EquipmentState PreviousState);

	UFUNCTION(Server, Reliable)
	void Server_SetEquipmentState(ECCD_EquipmentState NewState);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEquipMontage(FName SectionName, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopMontage();

	/** --- 내부 장비 로직 --- */
	void HandleEquipmentEffects(ECCD_EquipmentState NewState);
	void ProceedToEquip(ECCD_EquipmentState NewState);

	UFUNCTION()
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void BindMontageEndedDelegate();
	
	/** --- 물리 및 상호작용 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(EditAnywhere, Category = "Design")
	float InteractRange = 300.f;
	
	// 현재 잡고 있는 컴포넌트 (쓰레기 메쉬)
	UPROPERTY()
	class UPrimitiveComponent* GrabbedComponent;

	// 잡기/놓기 로직
	UFUNCTION(Server, Reliable)
	void Server_GrabObject(UPrimitiveComponent* ComponentToGrab, FName BoneName, FVector GrabLocation);

	UFUNCTION(Server, Reliable)
	void Server_ReleaseObject();
	
	/** --- (카메라) 회전값 복제 --- */
	// 서버에서 클라이언트로 복제될 변수
	UPROPERTY(Replicated)
	FRotator Rep_FirstPersonCameraRotation;

	// 서버에 회전값을 전달하는 RPC (FRotator를 넘깁니다)
	UFUNCTION(Server, Unreliable)
	void Server_SetFirstPersonCameraRotation(FRotator NewRotation);
};
