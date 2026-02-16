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
class UCCD_InteractionComponent;
class UCCD_ViewComponent;

UCLASS()
class CCD_API ACCDCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    /** --- 1. 라이프 사이클 및 엔진 오버라이드 --- */
    ACCDCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** --- 2. 입력 인터페이스 (입력 바인딩용) --- */
    UFUNCTION(BlueprintCallable, Category = "Interact")
    void PerformInteract();
    
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ToggleView();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SwitchToHands();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SwitchToMop();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SwitchToScanner();
    
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UseEquipment();

    /** --- 3. 애니메이션 및 멀티캐스트 (시각적 동기화) --- */
    UFUNCTION(NetMulticast, Reliable, Category = "Animation")
    void Multicast_PlayEquipMontage(FName SectionName, float PlayRate);

    UFUNCTION(NetMulticast, Reliable, Category = "Animation")
    void Multicast_StopMontage();

    UFUNCTION()
    void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void BindMontageEndedDelegate();
    
    /** --- 4. Getter / Setter --- */
    FORCEINLINE TObjectPtr<UStaticMeshComponent> GetMopMesh() const { return MopMesh; }
    FORCEINLINE TObjectPtr<UStaticMeshComponent> GetScannerMesh() const { return ScannerMesh; }
    FORCEINLINE TObjectPtr<UAnimMontage> GetEquipMontage() const { return EquipMontage; }
    
    FORCEINLINE bool GetIsUnequipping() const { return bIsUnequipping; }
    FORCEINLINE void SetIsUnequipping(bool bNewIsUnequipping) { bIsUnequipping = bNewIsUnequipping; }
    
    FORCEINLINE bool GetIsActionInProgress() const { return bIsActionInProgress; }
    FORCEINLINE void SetIsActionInProgress(bool bNewIsActionInProgress) { bIsActionInProgress = bNewIsActionInProgress; }

    FORCEINLINE UMaterialInstanceDynamic* GetMopMaterial() const { return MopMaterial; }
    
    FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE void SetRemoteControlRotation(FRotator NewRotation) { RemoteControlRotation = NewRotation;}
    
protected:
    /** --- 5. 라이프 사이클 내부 로직 --- */
    virtual void BeginPlay() override;
    
    /** --- 6. 컴포넌트 --- */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USceneComponent> CameraRoot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TObjectPtr<UStaticMeshComponent> MopMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TObjectPtr<UStaticMeshComponent> ScannerMesh;
    
    /** --- 캐릭터 기능성 컴포넌트 --- */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCCD_ViewComponent> ViewComp;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCCD_InteractionComponent> InteractionComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCCD_EquipmentComponent> EquipmentComp;
    
    /** --- 7. 서버 권한 로직 (RPC) --- */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation")
    void Server_PlayActionOfMop();

    UFUNCTION(Server, Reliable, Category = "Movement")
    void Server_SetMaxWalkSpeed(float NewSpeed);

    /** --- 8. 상태 변수 및 복제 데이터 --- */
    UPROPERTY(Replicated)
    FRotator RemoteControlRotation;

    bool bIsUnequipping = false;
    bool bIsActionInProgress = false;

    /** --- 9. 내부 헬퍼 함수 --- */
    void PerformCleaningTrace() const;
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(float NewSpeed);

    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> EquipMontage;

    UPROPERTY(EditAnywhere, Category = "Design")
    float InteractRange = 300.f;
    
private: 
    UPROPERTY()
    UMaterialInstanceDynamic* MopMaterial;
};