
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCDCharacter.generated.h"

// --- 전방 선언 ---
class UCameraComponent;
class USpringArmComponent;
class UPhysicsHandleComponent;
class UAnimMontage;

class UCCD_EquipmentComponent;

UCLASS()
class CCD_API ACCDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACCDCharacter();
	virtual void Tick(float DeltaTime) override;
	void UpdatePhysicsHandleTarget() const;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** --- 장비 전환 및 뷰 모드 --- */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToHands();
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToMop();
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToScanner();
	
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleView();
	UFUNCTION(Server, Reliable)
	void Server_ToggleView(bool bNewIsFirstPerson);		// ToggleView
	void ApplyViewMode(bool bFirstPerson);				// ToggleView
	
	/** --- 상호작용 및 물리 핸들 --- */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PerformInteract();
	UFUNCTION(Server, Reliable)
	void Server_PerformInteract();	// PerformInteract RPC
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PerformCleaningTrace();
	
	/** --- 몽타주 제어 및 델리게이트 --- */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation")
	void Server_PlayActionOfMop();	// 대걸레 액션 재생 요청 및 라인 트레이스 호출
	
	UFUNCTION(NetMulticast, Reliable, Category = "Animation")
	void Multicast_PlayEquipMontage(FName SectionName, float PlayRate);	// 장비 장착 몽타주 재생
	UFUNCTION(NetMulticast, Reliable, Category = "Animation")
	void Multicast_StopMontage();
	UFUNCTION()
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted); // 몽타주 종료 델리게이트 콜백
	void BindMontageEndedDelegate();
	
	/** --- Getter / Setter --- */
	TObjectPtr<UStaticMeshComponent> GetMopMesh() const { return MopMesh; }
	TObjectPtr<UStaticMeshComponent> GetScannerMesh() const { return ScannerMesh; }
	bool GetIsUnequipping() const { return bIsUnequipping; }
	void SetIsUnequipping(bool bNewIsUnequipping) { bIsUnequipping = bNewIsUnequipping; }
	bool GetIsActionInProgress() const { return bIsActionInProgress; }
	void SetIsActionInProgress(bool bNewIsActionInProgress) { bIsActionInProgress = bNewIsActionInProgress; }
	TObjectPtr<UAnimMontage> GetEquipMontage() const { return EquipMontage; }
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 컴포넌트 --- */
	/** --- 장비 컴포넌트 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UCCD_EquipmentComponent> EquipmentComp;
	
	/** --- 카메라 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Camera")
	bool bIsFirstPerson = false;
	
	/** --- 장비 및 메시 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MopMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ScannerMesh;
	
	/** --- 애니메이션 몽타주 제어 --- */
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;	// 장비 교체 몽타주
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsActionInProgress = false;
	bool bIsUnequipping = false;
	
	/** --- 물리 및 상호작용 --- */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;
	
	UPROPERTY(EditAnywhere, Category = "Design")
	float InteractRange = 300.f;
	
	// 현재 잡고 있는 컴포넌트 (쓰레기 메쉬)
	UPROPERTY(Replicated)
	class UPrimitiveComponent* GrabbedComponent;

	// 잡기/놓기 로직
	UFUNCTION(Server, Reliable)
	void Server_GrabObject(UPrimitiveComponent* ComponentToGrab, FName BoneName, FVector GrabLocation);
	
	UFUNCTION(Server, Reliable)
	void Server_ReleaseObject();
	
	/** --- (카메라) 회전값 복제 --- */
	// 카메라 회전값 복제 변수
	UPROPERTY(Replicated)
	FRotator Rep_FirstPersonCameraRotation;

	UFUNCTION(Server, Unreliable)
	void Server_SetFirstPersonCameraRotation(FRotator NewRotation);
	
	// 리모트 컨트롤 회전값 복제 변수 -> 고개를 까닥이는 모션용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FRotator RemoteControlRotation;

	UFUNCTION(Server, Unreliable)
	void Server_SetControlRotation(FRotator NewRotation);
	
	// 마지막으로 서버에 전송했던 회전값 기록
	FRotator LastSentRotation;

	// 동기화를 수행할 최소 각도 차이 (임계값)
	const float RotationThreshold = 0.1f;
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetRunning(float NewSpeed);
	UFUNCTION(Server, Reliable, Category = "Movement")
	void Server_SetMaxWalkSpeed(float NewSpeed);
};
