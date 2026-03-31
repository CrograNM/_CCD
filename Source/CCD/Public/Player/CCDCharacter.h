#pragma once

#include "CoreMinimal.h"
#include "Component/CCD_StatComponent.h"
#include "GameFramework/Character.h"
#include "CCDCharacter.generated.h"

class ADecal_StainActor_Base;
class UCameraComponent;
class USpringArmComponent;
class UPhysicsHandleComponent;
class UAnimMontage;
class UCCD_InteractionComponent;
class UCCD_ViewComponent;
class UCCD_EquipmentComponent;
class AGeometryCollectionActor;
class ACCD_BodyFragment;
class UInputAction;

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
    
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(bool bNewIsRunning);
    
    UFUNCTION(BlueprintCallable, Category = "Eye")
    void CloseEye();
    
    void DestroyAllEquipment() const;
    
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
    UFUNCTION(BlueprintCallable, Category = "Animation | Emote")
    void PerformEmote(FName EmoteSection);
    
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation | Emote")
    void Server_PlayEmoteMontage(FName EmoteSection);
    
    UFUNCTION(NetMulticast, Reliable, Category = "Animation | Emote")
    void Multicast_PlayEmoteMontage(FName SectionName, float PlayRate);
    
    UFUNCTION(NetMulticast, Reliable, Category = "Animation")
    void Multicast_PlayEquipMontage(FName SectionName, float PlayRate);
    
    UFUNCTION(NetMulticast, Reliable, Category = "Animation")
    void Multicast_StopMontage();
    
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation")
    void Server_StopMontage();
    
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Animation")
    void Server_PlayActionOfMop();
    
    UPROPERTY()
    FName CurrentEmoteSection; // 현재 재생 중인 이모트 섹션 이름 (없으면 NAME_None)
    
    /** --- Montage End CallBack Binding --- */
    void BindMontageEndedDelegate();
    UFUNCTION()
    void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    UFUNCTION()
    void OnEmoteMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    
    /** --- Getter / Setter --- */
    FORCEINLINE TObjectPtr<UAnimMontage> GetEquipMontage() const { return EquipMontage; }
    
    FORCEINLINE bool GetIsUnequipping() const { return bIsUnequipping; }
    FORCEINLINE void SetIsUnequipping(bool bNewIsUnequipping) { bIsUnequipping = bNewIsUnequipping; }
    
    FORCEINLINE bool GetIsActionInProgress() const { return bIsActionInProgress; }
    FORCEINLINE void SetIsActionInProgress(bool bNewIsActionInProgress) { bIsActionInProgress = bNewIsActionInProgress; }

    FORCEINLINE bool GetIsEmoting() const { return bIsEmoting; }
    FORCEINLINE void SetIsEmoting(bool bNewIsEmoting) { bIsEmoting = bNewIsEmoting; }
    
    FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    
    FORCEINLINE void SetRemoteControlRotation(FRotator NewRotation) { RemoteControlRotation = NewRotation;}
    UFUNCTION(BlueprintCallable)
    FORCEINLINE FRotator GetRemoteControlRotation() const { return RemoteControlRotation; }
    
    FORCEINLINE UCCD_ViewComponent* GetViewComp() const { return ViewComp; }
    FORCEINLINE UCCD_EquipmentComponent* GetEquipmentComp() const { return EquipmentComp; }
    
    UFUNCTION(BlueprintCallable)
    FORCEINLINE bool IsDead() const { return bIsDead; }
    
    bool GetIsObserveActivated() const { return StatComp ? StatComp->GetIsObserveActivated() : false; }
    
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
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCCD_StatComponent> StatComp;
    
    /** --- 기타 --- */
    UPROPERTY(Replicated)
    FRotator RemoteControlRotation;
    bool bIsUnequipping = false;
    bool bIsActionInProgress = false; // 애니메이션, 상호작용 등 액션 진행 중인지 여부
    
    UPROPERTY(Replicated)
    bool bIsEmoting = false; // 이모트 중인지 여부
    UPROPERTY(Replicated)
    bool bPendingEmote = false; // 장비 교체 중에 이모트 재생 요청이 들어왔는지 여부 (장비 교체 -> 이후 이모트 재생)
    
    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> EquipMontage;
    
    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> EmoteMontage;
    
    UPROPERTY(EditAnywhere, Category = "Design")
    float InteractRange = 300.f;
    
    /** --- 사망 상태 관리 --- */
    UPROPERTY(ReplicatedUsing = OnRep_IsDead)
    bool bIsDead = false;
    
    // 사망 시 조각에 가할 충격의 세기
    UPROPERTY(EditAnywhere, Category = "Death")
    float DeathImpulseStrength = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Death")
    TSubclassOf<ACCD_BodyFragment> DeathFragmentClass;

    UPROPERTY(EditAnywhere, Category = "Death")
    TArray<TObjectPtr<USkeletalMesh>> FragmentMeshList;
    
    UPROPERTY(EditAnywhere, Category = "Death")
    float BloodSpawnHeight = 150.f;
    
    UPROPERTY(EditAnywhere, Category = "Death")
    float BloodSpawnRange = 300.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Death")
    TSubclassOf<ADecal_StainActor_Base> BloodStainActorClass;
    
    UFUNCTION()
    void OnRep_IsDead();
    void HandleDeath(); // 사망 시 서버에서 호출
    void HandleRevive(); // 부활 시 서버에서 호출
    
    // 사운드
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UPawnNoiseEmitterComponent* NoiseEmitter;
    
    UFUNCTION(Server, Reliable)
    void Server_Trigger096Panic(ACCD_096* Target096);
    
    // 마우스 이동 입력을 담당
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;
    
    // 마우스 좌클릭 입력을 담당
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> RotateAction;
    
    // 마우스 이동
    void Look(const struct FInputActionValue& Value);
    
    // 마우스 좌클릭 (회전 모드)
    void OnRotationPressed();
    void OnRotationReleased();
};