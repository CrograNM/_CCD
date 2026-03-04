#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCDCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPhysicsHandleComponent;
class UAnimMontage;
class UCCD_InteractionComponent;
class UCCD_ViewComponent;
class UCCD_EquipmentComponent;
class AGeometryCollectionActor;
class ACCD_BodyFragment;

UCLASS()
class CCD_API ACCDCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    /** --- 라이프 사이클 및 엔진 오버라이드 --- */
    ACCDCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    /** --- 입력 바인딩 함수 --- */
    UFUNCTION(BlueprintCallable, Category = "Interact")
    void PerformInteract();
    
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ToggleView();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SwitchEquipment(const ECCD_EquipmentState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UseEquipment();
    UFUNCTION(Server, Reliable, Category = "Equipment")
    void Server_UseEquipment();
    
    // 사망 처리
    UFUNCTION(BlueprintCallable, Category = "Death")
    void Die();
    UFUNCTION(Server, Reliable, Category = "Death")
    void Server_Die();
    
    // 부활 처리
    UFUNCTION(BlueprintCallable, Category = "Death")
    void Revive();
    UFUNCTION(Server, Reliable, Category = "Death")
    void Server_Revive();
    
    /** --- 애니메이션 및 동기화 --- */
    UFUNCTION(NetMulticast, Reliable, Category = "Animation")
    void Multicast_PlayEquipMontage(FName SectionName, float PlayRate);

    UFUNCTION(NetMulticast, Reliable, Category = "Animation")
    void Multicast_StopMontage();
    
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation")
    void Server_PlayActionOfMop();
    
    UFUNCTION(Server, Reliable, Category = "Movement")
    void Server_SetMaxWalkSpeed(float NewSpeed);
    
    UFUNCTION()
    void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void BindMontageEndedDelegate();
    
    /** --- Getter / Setter --- */
    FORCEINLINE TObjectPtr<UAnimMontage> GetEquipMontage() const { return EquipMontage; }
    
    FORCEINLINE bool GetIsUnequipping() const { return bIsUnequipping; }
    FORCEINLINE void SetIsUnequipping(bool bNewIsUnequipping) { bIsUnequipping = bNewIsUnequipping; }
    
    FORCEINLINE bool GetIsActionInProgress() const { return bIsActionInProgress; }
    FORCEINLINE void SetIsActionInProgress(bool bNewIsActionInProgress) { bIsActionInProgress = bNewIsActionInProgress; }

    FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE void SetRemoteControlRotation(FRotator NewRotation) { RemoteControlRotation = NewRotation;}
    
    FORCEINLINE UCCD_ViewComponent* GetViewComp() const { return ViewComp; }
    
    UFUNCTION(BlueprintCallable)
    FORCEINLINE bool IsDead() const { return bIsDead; }
    
    void CheckForSCP096();
    
protected:
    virtual void BeginPlay() override;
    
    /** --- 컴포넌트 --- */
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

    /** --- 캐릭터 기능성 컴포넌트 --- */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCCD_ViewComponent> ViewComp;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCCD_InteractionComponent> InteractionComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCCD_EquipmentComponent> EquipmentComp;
    
    /** --- 기타 --- */
    UPROPERTY(Replicated)
    FRotator RemoteControlRotation;
    bool bIsUnequipping = false;
    bool bIsActionInProgress = false;

    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> EquipMontage;

    UPROPERTY(EditAnywhere, Category = "Design")
    float InteractRange = 300.f;
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(float NewSpeed);
    
    /** --- 사망 상태 관리 --- */
    UPROPERTY(ReplicatedUsing = OnRep_IsDead)
    bool bIsDead = false;
    
    // 사망 시 조각에 가할 충격의 세기
    UPROPERTY(EditAnywhere, Category = "Death")
    float DeathImpulseStrength = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Death")
    TSubclassOf<ACCD_BodyFragment> DeathFragmentClass;

    UPROPERTY(EditAnywhere, Category = "Death")
    TArray<TObjectPtr<UStaticMesh>> FragmentMeshList;
    
    UFUNCTION()
    void OnRep_IsDead();
    void HandleDeath(); // 사망 시 서버에서 호출
    void HandleRevive(); // 부활 시 서버에서 호출
};