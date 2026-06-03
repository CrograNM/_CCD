#pragma once

#include "CoreMinimal.h"
#include "Component/CCD_EquipmentComponent.h"
#include "Component/CCD_InteractionComponent.h"
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
class UUserWidget;

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
    
    FORCEINLINE bool IsInvincible() const { return bIsInvincible; }
    
    /** --- 애니메이션 및 동기화 --- */
    UFUNCTION(BlueprintCallable, Category = "Animation | Emote")
    void PerformEmote(FName EmoteSection);
    
    UFUNCTION(Server, Reliable)
    void Server_PerformEmote(FName EmoteSection);
    
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
    FORCEINLINE UCCD_InteractionComponent* GetInteractionComp() const { return InteractionComp; }
    
    FORCEINLINE bool GetIsEquipHand() const { return EquipmentComp ? EquipmentComp->GetEquipmentState() == ECCD_EquipmentState::EES_Hands : true; }
    UFUNCTION(BlueprintCallable)
    FORCEINLINE bool IsDead() const { return bIsDead; }
    
    bool GetIsObserveActivated() const { return StatComp ? StatComp->GetIsObserveActivated() : false; }
    
    void CheckForSCP096();
    
    FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetMesh1P() const { return Mesh1P; }
    void SetMesh1PVisibility(bool bVisible);
    
    bool GetIsGrabbed() const;
    
    FORCEINLINE TSubclassOf<ADecal_StainActor_Base> GetBloodStainActorClass() const { return BloodStainActorClass; }
    
    bool IsRotationMode() const { return InteractionComp ? InteractionComp->IsRotationMode() : false; }

    /** 캐릭터의 커스텀 이름 (UI용) */
    UFUNCTION(BlueprintCallable, Category = "Player")
    FString GetPlayerCustomName() const;
    
    // 피 묻은 발자국 설정
    void AddBloodToFeet(int32 StepCount);

    // AnimNotify에서 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void TrySpawnFootprint(FName FootSocketName);
    
    UFUNCTION(Server, Reliable)
    void Server_SpawnFootprint(FVector Location, FRotator Rotation, bool bIsLeft);
    
    // 939 전용 데미지 처리 함수 오버라이드
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    
    // 수류탄을 주웠을 때 보관할 포인터 세터
    FORCEINLINE void SetHeldGrenade(class ACCD_FreezeGrenade* Grenade) { HeldGrenade = Grenade; }

    // 현재 수류탄을 들고 있는지 여부 반환
    FORCEINLINE bool HasFreezeGrenade() const { return HeldGrenade != nullptr; }
    
protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<USkeletalMeshComponent> Mesh1P; // FPS용 1인칭 메쉬
    
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
    
    UPROPERTY(Replicated)
    bool bIsUnequipping = false;
    
    UPROPERTY(Replicated)
    bool bIsActionInProgress = false; // 애니메이션, 상호작용 등 액션 진행 중인지 여부
    
    UPROPERTY(Replicated)
    bool bIsEmoting = false; // 이모트 중인지 여부
    
    UPROPERTY(Replicated)
    bool bPendingEmote = false; // 장비 교체 중에 이모트 재생 요청이 들어왔는지 여부 (장비 교체 -> 이후 이모트 재생)
    
    UPROPERTY(Replicated)
    FName CurrentEmoteSection; // 현재 재생 중인 이모트 섹션 이름 (없으면 NAME_None)
    
    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> EquipMontage;
    
    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> EmoteMontage;
    
    // 플레이어 행동 소음 관련
    virtual void Landed(const FHitResult& Hit) override;
    void MakeFootstepNoise(float LoudnessMultiplier = 1.0f);
    
    /** --- 사망 상태 관리 --- */
    UPROPERTY(ReplicatedUsing = OnRep_IsDead)
    bool bIsDead = false;
    
    UPROPERTY(Replicated)
    bool bIsInvincible = false; // 무적 상태 여부

    FTimerHandle InvincibilityTimerHandle; // 무적 해제용 타이머

    void DeactivateInvincibility(); // 무적 해제 함수
    
    /** 부활 대기 시간 설정 */
    UPROPERTY(EditAnywhere, Category = "Design | Death")
    float RespawnDelay = 10.0f;
    
    void CheckAndRespawn();

    FTimerHandle RespawnTimerHandle;
    
    // 사망 시 조각에 가할 충격의 세기
    UPROPERTY(EditAnywhere, Category = "Design | Death")
    float DeathImpulseStrength = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Design | Death")
    TSubclassOf<ACCD_BodyFragment> DeathFragmentClass;

    UPROPERTY(EditAnywhere, Category = "Design | Death")
    TArray<TObjectPtr<USkeletalMesh>> FragmentMeshList;
    
    UPROPERTY(EditAnywhere, Category = "Design | Death")
    float BloodSpawnHeight = 150.f;
    
    UPROPERTY(EditAnywhere, Category = "Design | Death")
    float BloodSpawnRange = 300.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Design | Death")
    TSubclassOf<ADecal_StainActor_Base> BloodStainActorClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "Design | Death")
    TArray<TSubclassOf<ACCD_BodyFragment>> InternalOrganFragmentClasses;
    
    UPROPERTY(Replicated)
    int32 RemainingFootprints = 0;

    // 발자국으로 사용할 데칼 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Design | VFX")
    TSubclassOf<ADecal_StainActor_Base> FootprintLeftDecalClass;

    UPROPERTY(EditDefaultsOnly, Category = "Design | VFX")
    TSubclassOf<ADecal_StainActor_Base> FootprintRightDecalClass;
    
    // 발자국 소리 메타 사운드
    UPROPERTY(EditAnywhere, Category = "Design | Sound")
    TObjectPtr<USoundBase> NormalFootstepSound;

    UPROPERTY(EditAnywhere, Category = "Design | Sound")
    TObjectPtr<USoundBase> BloodyFootstepSound;
   
    
    UFUNCTION()
    void OnRep_IsDead();
    void HandleDeath(); // 사망 시 서버에서 호출
    void HandleRevive(); // 부활 시 서버에서 호출
    
    // 사운드
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UPawnNoiseEmitterComponent* NoiseEmitter;
    
    UPROPERTY(EditAnywhere, Category = "Design | Sound")
    TObjectPtr<USoundBase> NormalLandingSound;

    UPROPERTY(EditAnywhere, Category = "Design | Sound")
    TObjectPtr<USoundBase> BloodyLandingSound;
    
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
    
    // 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Design | Stat")
    float MaxHealth = 3.0f;
    
    // 현재 체력
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentHealth, Category = "Design | Stat")
    float CurrentHealth;
    
    // 클라이언트에서 체력이 변했을 때 호출될 함수 (UI 업데이트용)
    UFUNCTION()
    void OnRep_CurrentHealth();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Design | UI")
    void OnDamageEffectTriggered(float CurrentHealthRatio);
    
    // 피격 이펙트 관련
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Design | UI")
    TSubclassOf<class UUserWidget> DamageWidgetClass;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Design | UI")
    TObjectPtr<class UUserWidget> DamageWidgetInstance;
    
    // 캐릭터 피격(부상) 사운드
    UPROPERTY(EditAnywhere, Category = "Design | Sound")
    TObjectPtr<USoundBase> HurtSound;
    
    // 현재 플레이어가 E키로 획득하여 들고 있는 수류탄 액터 포인터
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Design | Equipment")
    TObjectPtr<class ACCD_FreezeGrenade> HeldGrenade;

    // 수류탄 투척 서버 RPC 함수
    UFUNCTION(Server, Reliable)
    void Server_ThrowHeldGrenade(FVector LaunchDir);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Design | Equipment")
    TSubclassOf<class ACCD_FreezeGrenade> FreezeGrenadeClass;
};