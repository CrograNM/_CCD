
#include "Player/CCDCharacter.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "Player/CCDPlayerController.h"
#include "Actor/CCD_BodyFragment.h"
#include "Actor/Decal_StainActor_Base.h"
#include "AI/CCD_096.h"
#include "Components/BoxComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Component/CCD_EquipmentComponent.h"
#include "Component/CCD_InteractionComponent.h"
#include "Component/CCD_StatComponent.h"
#include "Component/CCD_ViewComponent.h"
#include "Component/WashableComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameData/CCDPlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MovieSceneSequenceID.h"
#include "Actor/CCD_FreezeGrenade.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameData/CCDGameMode.h"
#include "GameData/CCDGameState.h"

/** --- 생성자 및 기본 함수 --- */
ACCDCharacter::ACCDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// --- 기능성 컴포넌트 추가 ---
	ViewComp = CreateDefaultSubobject<UCCD_ViewComponent>(TEXT("ViewComp"));
	InteractionComp = CreateDefaultSubobject<UCCD_InteractionComponent>(TEXT("InteractionComp"));
	EquipmentComp = CreateDefaultSubobject<UCCD_EquipmentComponent>(TEXT("EquipmentComp"));
	StatComp = CreateDefaultSubobject<UCCD_StatComponent>(TEXT("StatComp"));
	
	// --- 카메라 설정 ---
	// 3인칭 카메라
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	
	// 1인칭 카메라
	CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
	CameraRoot->SetupAttachment(RootComponent); 
	CameraRoot->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight)); 

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(CameraRoot); 
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	// --- 물리 핸들 ---
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->SetIsReplicated(true);
	
	PhysicsHandle->LinearStiffness = 750.0f;  // 기본값은 보통 높음. 500~1000 사이로 조절
	PhysicsHandle->AngularStiffness = 750.0f; // 회전 지연 정도
	PhysicsHandle->LinearDamping = 50.0f;     // 출렁임을 방지하기 위한 감쇠
	PhysicsHandle->AngularDamping = 50.0f;
	PhysicsHandle->InterpolationSpeed = 20.0f; // 핸들 자체의 내부 보간 속도
	
	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
	
	// --- 1인칭 팔 메쉬 ---
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetupAttachment(FirstPersonCamera);		// 카메라에 붙여야 카메라 회전에 따라 팔이 자연스럽게 움직입니다.
	Mesh1P->bOnlyOwnerSee = true;						// 자신(Owner)에게만 보임
	Mesh1P->bCastDynamicShadow = false;					// 팔 자체 그림자는 필요 없음 (전신 메쉬가 대신 맺어줌)
	Mesh1P->CastShadow = false;
	// 기존 메쉬 설정 변경
	GetMesh()->SetOwnerNoSee(true);            // 자신에게는 전신 메쉬가 안 보이게 함
	GetMesh()->bCastHiddenShadow = true;       // 메쉬가 숨겨져 있어도 그림자는 맺히게 함 (중요)
}

void ACCDCharacter::SetMesh1PVisibility(bool bVisible)
{
	if (Mesh1P)
	{
		Mesh1P->SetVisibility(bVisible);
	}
}
bool ACCDCharacter::GetIsGrabbed() const
{
	if (InteractionComp)
	{
		return InteractionComp->GetGrabbedComponent() != nullptr;
	}
	return false;
}
FString ACCDCharacter::GetPlayerCustomName() const
{
	if (ACCDPlayerState* PS = GetPlayerState<ACCDPlayerState>())
	{
		FString CustomName = PS->CustomName;
		if (IsLocallyControlled()) CustomName += TEXT(" (You)");
		
		return CustomName;
	}
	return TEXT("Unknown Player");
}

void ACCDCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (Mesh1P && GetMesh())
	{
		Mesh1P->SetLeaderPoseComponent(GetMesh());
	}
	
	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}
void ACCDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 살아있을 때만 시선 체크 수행
	if (!bIsDead)
	{
		CheckForSCP096();
		
		// ================= [SCP-939 실내 거리 디버깅 로직 추가] =================
		if (IsLocallyControlled() && GetWorld())
		{
			TArray<AActor*> Found939Actors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Found939Actors);

			for (AActor* TargetActor : Found939Actors)
			{
				if (TargetActor && TargetActor->GetName().Contains(TEXT("939")))
				{
					// 캐릭터와 SCP-939 사이의 거리 계산
					float DistanceInCm = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
					float DistanceInMeters = DistanceInCm / 100.0f; // cm -> m 변환
					
					float CurrentLoudness = 0.3f; // 기본 걷기
					FString StateStr = TEXT("Walking (듣기범위: 7.5m)");

					if (StatComp && StatComp->GetIsRunning())
					{
						CurrentLoudness = 1.0f;
						StateStr = TEXT("Sprinting (듣기범위: 25m)");
					}
					
					FString DebugMessage = FString::Printf(
						TEXT("[DEBUG] SCP-939와의 거리: %.2fm | 현재 상태: %s | Loudness: %.1f"), 
						DistanceInMeters, *StateStr, CurrentLoudness
					);
					
					GEngine->AddOnScreenDebugMessage(10, 0.0f, FColor::Yellow, DebugMessage);
				}
			}
		}
		// ====================================================================
	}
}
void ACCDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC)
	{
		// 1. 마우스 이동 (LookAction은 에디터에서 할당된 InputAction)
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACCDCharacter::Look);

		// 2. 마우스 좌클릭 (IA_RotateMode 등의 이름으로 에디터에서 생성 필요)
		EIC->BindAction(RotateAction, ETriggerEvent::Started, this, &ACCDCharacter::OnRotationPressed);
		EIC->BindAction(RotateAction, ETriggerEvent::Completed, this, &ACCDCharacter::OnRotationReleased);
	}
}
void ACCDCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 변수 복제 등록
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDCharacter, RemoteControlRotation);
	DOREPLIFETIME(ACCDCharacter, bIsDead);
	DOREPLIFETIME(ACCDCharacter, bIsEmoting);
	DOREPLIFETIME(ACCDCharacter, bPendingEmote);
	DOREPLIFETIME(ACCDCharacter, bIsActionInProgress);
	DOREPLIFETIME(ACCDCharacter, bIsUnequipping);
	DOREPLIFETIME(ACCDCharacter, CurrentEmoteSection);
	DOREPLIFETIME(ACCDCharacter, RemainingFootprints);
	DOREPLIFETIME(ACCDCharacter, bIsInvincible);
	DOREPLIFETIME(ACCDCharacter, MaxHealth);
	DOREPLIFETIME(ACCDCharacter, CurrentHealth);
	DOREPLIFETIME(ACCDCharacter, HeldGrenade);
}

void ACCDCharacter::PerformEmote(FName EmoteSection)
{
	Server_PerformEmote(EmoteSection);
}
void ACCDCharacter::Server_PerformEmote_Implementation(FName EmoteSection)
{
	if (!EmoteMontage || bIsDead) return;
	if (bIsEmoting && CurrentEmoteSection == EmoteSection) return;
		
	CurrentEmoteSection = EmoteSection;
	UE_LOG(LogTemp, Warning, TEXT("[ACCDCharacter] Server_PerformEmote called with section: %s"), *CurrentEmoteSection.ToString());
	
	// 상태 초기화: 새로운 명령이 들어왔으므로 기존 예약이나 액션은 강제 중단
	if (bIsActionInProgress || bIsEmoting || bPendingEmote)
	{
		bPendingEmote = false;
		Server_StopMontage(); 
        
		bIsActionInProgress = false;
		bIsEmoting = false;
		bIsUnequipping = false;
	}
	if (EmoteMontage && EmoteMontage->GetSectionIndex(EmoteSection) == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_PerformEmote: Invalid Emote Section [%s] requested!"), *EmoteSection.ToString());
		return;
	}
	
	// 장비 장착 여부에 따라 -> 바로 재생 or 장비 해제 후 재생
	if (EquipmentComp && EquipmentComp->GetEquipmentState() != ECCD_EquipmentState::EES_Hands)
	{
		bPendingEmote = true; // 장비 해제 후 이모트 재생 예약
		SwitchEquipment(ECCD_EquipmentState::EES_Hands);
	}
	else
	{
		// 맨손이라면 즉시 이모트 재생
		Server_PlayEmoteMontage(CurrentEmoteSection);
	}
}
void ACCDCharacter::Server_PlayEmoteMontage_Implementation(FName EmoteSection)
{
	if (EmoteMontage && EmoteMontage->GetSectionIndex(EmoteSection) != INDEX_NONE)
	{
		bIsActionInProgress = true; // 액션 진행중 플래그 설정
		bIsEmoting = true; // 이모트 상태 설정
		Multicast_PlayEmoteMontage(EmoteSection, 1.0f);
	
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EmoteMontage)
		{
			FOnMontageEnded EmoteEndedDelegate;
			EmoteEndedDelegate.BindUObject(this, &ACCDCharacter::OnEmoteMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EmoteEndedDelegate, EmoteMontage);
		}
	}
	else
	{
		// 섹션이 없으면 상태 초기화
		bIsActionInProgress = false;
		bIsEmoting = false;
		UE_LOG(LogTemp, Error, TEXT("Server_PlayEmoteMontage: Invalid Emote Section [%s] requested!"), *EmoteSection.ToString());
	}
}
void ACCDCharacter::Multicast_PlayEmoteMontage_Implementation(FName SectionName, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EmoteMontage)
	{
		if (EmoteMontage->GetSectionIndex(SectionName) != INDEX_NONE)
		{
			AnimInstance->Montage_Play(EmoteMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(SectionName, EmoteMontage);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Emote Section [%s] not found in EmoteMontage!"), *SectionName.ToString());
		}
	}
}

/** --- 입력 바인딩 : 상호작용, 1-3인칭 전환, 장비 전환 및 사용 --- */
void ACCDCharacter::PerformInteract()
{
	if (bIsDead) return;
	// if (EquipmentComp->GetEquipmentState() != ECCD_EquipmentState::EES_Hands) return;
	if (InteractionComp) InteractionComp->PerformInteract();
}
void ACCDCharacter::SwitchEquipment(const ECCD_EquipmentState NewState)
{
	if (bIsDead) return;
	if (EquipmentComp) EquipmentComp->SwitchEquipment(NewState);
}
void ACCDCharacter::ToggleView()
{
	if (bIsDead) return;
	if (ViewComp) ViewComp->ToggleView(); 
}

void ACCDCharacter::SetViewModeFPS(bool bNewIsFirstPerson)
{
	if (bIsDead) return;
	if (ViewComp) ViewComp->SetViewModeFPS(bNewIsFirstPerson);
}

void ACCDCharacter::UseEquipment()
{
	if (bIsDead) return;
	Server_UseEquipment();
}
void ACCDCharacter::Server_UseEquipment_Implementation()
{
	if (EquipmentComp)
	{
		EquipmentComp->ExecuteActiveEquipment();
	}
}
void ACCDCharacter::SetRunning(bool bNewIsRunning)
{
	if (bIsDead) return;
	if (StatComp) StatComp->SetIsRunning(bNewIsRunning);
}

void ACCDCharacter::CloseEye()
{
	if (bIsDead) return;
	if (StatComp) StatComp->CloseEye();
}
void ACCDCharacter::DestroyAllEquipment() const
{
	if (!HasAuthority()) return;
	if (EquipmentComp)EquipmentComp->DestroyAllEquipment();
}

/** --- 사망 및 부활 처리 --- */
void ACCDCharacter::Die()
{	
	Server_Die();
}
void ACCDCharacter::Server_Die_Implementation()
{	
	if (bIsDead) return; // 이미 사망한 경우 중복 처리 방지
	UE_LOG(LogTemp, Warning, TEXT("[ACCDCharacter] Die called"));
	
	if (const ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->GetCurrentLives() <= 0)
		{
			if (ACCDGameState* GS = GM->GetGameState<ACCDGameState>())
			{
				if (!GS->bIsGameOver)
				{
					GS->bIsGameOver = true;
					
					// ================= [AI 정지 로직] =================
					if (UWorld* World = GetWorld())
					{
						for (TActorIterator<AAIController> It(World); It; ++It)
						{
							if (AAIController* AIController = *It)
							{
								if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
								{
									BrainComp->StopLogic(TEXT("Game Over"));
								}
								// 이동 컴포넌트도 명확하게 정지
								AIController->StopMovement();
							}
						}
					}
					
					if (HasAuthority())
					{
						GS->OnRep_IsGameOver();
					}
					return;
				}
			}
		}
	}
	
	bIsDead = true;
	if (ACCDPlayerState* PS = GetPlayerState<ACCDPlayerState>())
	{
		PS->bIsDead = true;
		
		// 현재 서버 시간 + 대기 시간을 저장
		PS->RespawnStartTime = GetWorld()->GetTimeSeconds();
		PS->RespawnEndTime = GetWorld()->GetTimeSeconds() + RespawnDelay;
	}
	HandleDeath();	
	OnRep_IsDead(); 
	
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACCDCharacter::CheckAndRespawn, RespawnDelay, false);
	if (ACCDPlayerController* PC = Cast<ACCDPlayerController>(GetController()))
	{
		PC->ApplyDeath(true);
	}
}
void ACCDCharacter::Revive()
{
	Server_Revive();
}
void ACCDCharacter::Server_Revive_Implementation()
{
	if (!bIsDead) return;
	UE_LOG(LogTemp, Warning, TEXT("[ACCDCharacter] Revive called"));
	bIsDead = false;
	if (ACCDPlayerState* PS = GetPlayerState<ACCDPlayerState>())
	{
		PS->bIsDead = false;
		PS->RespawnStartTime = -1.0f; // 부활 시 초기화
		PS->RespawnEndTime = -1.0f; // 부활 시 초기화
	}
	HandleRevive();
	if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
	{
		// GameMode에서 적절한 시작 지점을 찾아줍니다.
		if (AActor* StartSpot = GM->FindPlayerStart(GetController()))
		{
			SetActorLocationAndRotation(StartSpot->GetActorLocation(), StartSpot->GetActorRotation());
		}
	}
	OnRep_IsDead();
}

void ACCDCharacter::CheckAndRespawn()
{
	// 이미 부활한 경우 중복 처리 방지
	if (!bIsDead) return; 
	
	if (ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->RequestRespawn(this);
	}
}

/** --- 몽타주 제어 및 델리게이트 --- */
void ACCDCharacter::Server_PlayActionOfMop_Implementation()
{
	if (bIsActionInProgress) return;
	
	// 실제 재생 여부 더블 체크
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(EquipMontage)) return;
	
	if (EquipmentComp->GetEquipmentState() == ECCD_EquipmentState::EES_Mop)
	{
		bIsActionInProgress = true;	// 액션 진행중 플래그 설정
		Multicast_PlayEquipMontage(TEXT("SwingMop"), 1.5f);	// 대걸레 휘두르기 애니메이션 재생
		BindMontageEndedDelegate();	// 몽타주 종료 델리게이트 바인딩
	}
}
void ACCDCharacter::Multicast_PlayEquipMontage_Implementation(FName SectionName, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage, PlayRate);
        
		if (PlayRate < 0.f)
		{
			int32 SectionIndex = EquipMontage->GetSectionIndex(SectionName);
			float SectionStartTime, SectionEndTime;
			EquipMontage->GetSectionStartAndEndTime(SectionIndex, SectionStartTime, SectionEndTime);
            
			AnimInstance->Montage_SetPosition(EquipMontage, SectionEndTime - 0.01f);
		}
		else
		{
			AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
		}
	}
}
void ACCDCharacter::Multicast_StopMontage_Implementation()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.2f);
	}
}
void ACCDCharacter::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!HasAuthority()) return;
	
	bIsActionInProgress = false; // 액션 상태 해제
	if (bInterrupted) bIsUnequipping = false;
	
	// 장비 해제(Unequipping) 단계가 끝났을 때
	if (bIsUnequipping)
	{
		EquipmentComp->ProceedToEquip(EquipmentComp->GetPendingEquipmentState());
	}
	
	// 이모트 예약 헀다면 (장비 교체 -> 이후 이모트 재생)
	if (bPendingEmote)
	{
		bPendingEmote = false; // 예약 해제
		Server_PlayEmoteMontage(CurrentEmoteSection);
	}
}
void ACCDCharacter::OnEmoteMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!HasAuthority()) return;
	SetViewModeFPS(true);
	bIsActionInProgress = false; // 액션 상태 해제
	bIsEmoting = false; // 이모트 상태 해제
}

void ACCDCharacter::Server_StopMontage_Implementation()
{
	SetViewModeFPS(true);
	Multicast_StopMontage();
}

void ACCDCharacter::BindMontageEndedDelegate()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ACCDCharacter::OnEquipMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, EquipMontage);
	}
}

/** --- 사망 상태 관리 --- */
void ACCDCharacter::OnRep_IsDead()
{
	if (bIsDead)
	{
		if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
			RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
		
		// 캐릭터 메쉬 숨김 및 충돌 비활성화
		if (GetMesh())
		{
			GetMesh()->SetHiddenInGame(true);
			GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
		}

		// 3인칭 시점으로 강제 전환 및 고정
		if (ViewComp) ViewComp->ApplyViewMode(false);
	}
	else
	{
		if (GetMesh())
		{
			GetMesh()->SetHiddenInGame(false);
			GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
		}
		if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
		{
			RootPrim->SetCollisionProfileName(TEXT("Pawn"));
			RootPrim->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
		}
		if (ViewComp) ViewComp->ApplyViewMode(ViewComp->GetIsFirstPerson());
	}
	
	// 사망자 본인 처리
	if (IsLocallyControlled())
	{
		if (ACCDPlayerController* PC = Cast<ACCDPlayerController>(GetController()))
		{
			if (!bIsDead) PC->ResetPlayerController(this);
			// else PC->ApplyDeath(bIsDead);
		}
	}
	
	// 관전자 처리 
	else if (ACCDPlayerController* LocalPC = Cast<ACCDPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (LocalPC->GetCurrentSpectateTarget() == this)
		{
			LocalPC->UpdateSpectatorWidget(this);
		}
	}
}
void ACCDCharacter::HandleDeath()
{
	if (!HasAuthority()) return;
	
	// 캡슐 컴포넌트 충돌 비활성화
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrim->SetCollisionProfileName(TEXT("NoCollision"));
		RootPrim->SetCanEverAffectNavigation(false); // 길 찾기 방해 금지
	}
	
	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	}
	
	// 물리 상호작용 정리 (잡고 있던 물체 투하)
	if (InteractionComp) InteractionComp->ForceRelease();
	
	// 장비 정리 (장비 액터 파괴)
	if (EquipmentComp)EquipmentComp->DestroyAllEquipment();
	
	// 이동 능력 상실
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}
	
	// 핏자국 데칼 스폰
	if (BloodStainActorClass)
	{
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		
		for (int i = 0; i < 1; ++i)
		{
			FHitResult HitResult;
			FVector Start = GetActorLocation() + FVector(0.f, 0.f, BloodSpawnHeight);
			FVector End = Start + FVector(FMath::RandRange(-BloodSpawnRange, BloodSpawnRange), FMath::RandRange(-BloodSpawnRange, BloodSpawnRange), -500.0f - BloodSpawnHeight);
			
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
				// 바닥 평면에 맞춘 회전값 (바닥 노멀 기준)
				FRotator SpawnRot = HitResult.ImpactNormal.Rotation();
				SpawnRot.Pitch -= 90.0f; // 데칼은 기본적으로 X축 방향으로 쏘므로 아래를 향하게 조정
		
				if (ADecal_StainActor_Base* SpawnedDecal = GetWorld()->SpawnActor<ADecal_StainActor_Base>(
					BloodStainActorClass, 
					HitResult.Location + HitResult.ImpactNormal * 1.5f, 
					SpawnRot, 
					SpawnParams))
				{
					if (UDecalComponent* DecalComp = SpawnedDecal->GetDecal())
					{
						DecalComp->SortOrder = i;
					}
				}
			}
		}
	}
	
	// 카오스 디스트럭션 적용 (Geometry Collection Mesh 스폰)
	if (DeathFragmentClass && FragmentMeshList.Num() > 0)
	{
		TArray<TObjectPtr<USkeletalMesh>> MeshesToSpawn;
		TArray<TObjectPtr<USkeletalMesh>> TempPool = FragmentMeshList;	// 원본 복사 풀
		int32 SpawnTargetCount = FMath::Min(3, TempPool.Num());		// 전체 개수와 3개 중 더 작은 값을 타겟 스폰 수로 결정
		
		for (int32 i = 0; i < SpawnTargetCount; ++i)
		{
			int32 RandomIndex = FMath::RandRange(0, TempPool.Num() - 1);
			if (TempPool[RandomIndex])
			{
				MeshesToSpawn.Add(TempPool[RandomIndex]);
			}
			// 중복 방지를 위해 선택된 요소를 스왑 후 제거
			TempPool.RemoveAtSwap(RandomIndex);
		}
		
		for (USkeletalMesh* FragmentMesh : MeshesToSpawn)
		{
			if (!FragmentMesh) continue;
			FVector SpawnLocation = GetActorLocation() + 
				FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
            
			ACCD_BodyFragment* Fragment = GetWorld()->SpawnActor<ACCD_BodyFragment>(
				DeathFragmentClass, 
				SpawnLocation, 
				GetActorRotation() + FRotator(0.f, -90.f, 0.f)
			);

			if (Fragment)
			{
				if (Fragment->GetMeshComp())
				{
					// 파편 메쉬 컴포넌트가 플레이어 본체(this)를 이동 시 무시
					Fragment->GetMeshComp()->IgnoreActorWhenMoving(this, true);
				}
    
				this->MoveIgnoreActorAdd(Fragment);
				
				// 방사형 패턴으로 충격파
				FBox SphereBounds = FragmentMesh->GetImportedBounds().GetBox();
				FVector MeshRelativeCenter = SphereBounds.GetCenter();
				FVector ImpulseDir = MeshRelativeCenter.GetSafeNormal();
				ImpulseDir.Z += FMath::RandRange(0.1f, 0.3f); // 위로도 약간 튀어오르게 (살짝 랜덤)
				ImpulseDir.Normalize();
				FVector FinalImpulse = ImpulseDir * (DeathImpulseStrength + FMath::RandRange(100.0f, 500.0f)); // 충격 세기에 약간의 랜덤 추가
				Fragment->InitFragment(FragmentMesh, FinalImpulse);
			}
		}
	}
	
	if (InternalOrganFragmentClasses.Num() > 0)
	{
		const int32 RandomIndex = FMath::RandHelper(InternalOrganFragmentClasses.Num());
		if (const TSubclassOf<ACCD_BodyFragment> SelectedDecalClass = InternalOrganFragmentClasses[RandomIndex])
		{
			ACCD_BodyFragment* SpawnedOrgan = GetWorld()->SpawnActor<ACCD_BodyFragment>(
				SelectedDecalClass, 
				GetActorLocation(), 
				GetActorRotation()
				);

			if (SpawnedOrgan)
			{
				if (SpawnedOrgan->GetMeshComp())
				{
					SpawnedOrgan->GetMeshComp()->IgnoreActorWhenMoving(this, true);
				}
				this->MoveIgnoreActorAdd(SpawnedOrgan);
				USkeletalMesh* FragmentMesh = SpawnedOrgan->GetMeshComp()->GetSkeletalMeshAsset();
				FBox SphereBounds = FragmentMesh->GetImportedBounds().GetBox();
				FVector MeshRelativeCenter = SphereBounds.GetCenter();
				FVector ImpulseDir = MeshRelativeCenter.GetSafeNormal();
				ImpulseDir.Z += FMath::RandRange(0.1f, 0.3f); // 위로도 약간 튀어오르게 (살짝 랜덤)
				ImpulseDir.Normalize();
				FVector FinalImpulse = ImpulseDir * (DeathImpulseStrength + FMath::RandRange(100.0f, 500.0f)); // 충격 세기에 약간의 랜덤 추가
				SpawnedOrgan->InitFragment(FragmentMesh, FinalImpulse);
			}
		}
	}
	
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAIController> It(World); It; ++It)
		{
			if (AAIController* AIC = *It)
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					if (BB->GetValueAsObject(TEXT("TargetActor")) == this)
					{
						BB->ClearValue(TEXT("TargetActor"));
						AIC->ClearFocus(EAIFocusPriority::Gameplay);
					}
				}
			}
		}
	}
}

void ACCDCharacter::HandleRevive()
{
	// 충돌 복구 및 네비게이션 영향 허용
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrim->SetCollisionProfileName(TEXT("Pawn"));
		RootPrim->SetCanEverAffectNavigation(true);
	}
	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	}

	// 이동 능력 복구
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetComponentTickEnabled(true);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	// 장비 재초기화
	if (EquipmentComp)
	{
		EquipmentComp->InitializeEquipment();
	}
	
	if (HasAuthority())
	{
		bIsInvincible = true;
        
		RemainingFootprints = 0;
		
		CurrentHealth = MaxHealth;
		
		// 3초 뒤에 무적 해제
		GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, this, &ACCDCharacter::DeactivateInvincibility, 3.0f, false);
		
		UE_LOG(LogTemp, Warning, TEXT("Player is Invincible for 3 seconds"));
	}
}
void ACCDCharacter::DeactivateInvincibility()
{
	if (HasAuthority())
	{
		bIsInvincible = false;
		UE_LOG(LogTemp, Warning, TEXT("Invincibility Expired."));
	}
}

// SCP 상호작용
void ACCDCharacter::CheckForSCP096()
{
	if (!IsLocallyControlled() || !FirstPersonCamera) return;
	
	FVector CameraLoc = FirstPersonCamera->GetComponentLocation();
	FVector CameraForward = FirstPersonCamera->GetForwardVector();
	
	TArray<AActor*> FoundSCPs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACCD_096::StaticClass(), FoundSCPs);

	for (AActor* Actor : FoundSCPs)
	{
		ACCD_096* SCP096 = Cast<ACCD_096>(Actor);
		if (!SCP096 || SCP096->IsPlayerMarked(this)) continue;
		
		FVector FaceLoc = SCP096->GetFaceTrigger()->GetComponentLocation();
		FVector DirToFace = (FaceLoc - CameraLoc).GetSafeNormal();
		
		float DotProduct = FVector::DotProduct(CameraForward, DirToFace);
		
		// 이 값이 클수록 더 정확히 쳐다봐야 함
		float ViewThreshold = 0.95f; 

		if (DotProduct > ViewThreshold)
		{
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(SCP096); 

			bool bIsOccluded = GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, FaceLoc, ECC_Visibility, Params);

			if (!bIsOccluded)
			{
				Server_Trigger096Panic(SCP096); 
			}
		}
	}
}
void ACCDCharacter::Server_Trigger096Panic_Implementation(ACCD_096* Target096)
{
	if (Target096)
	{
		Target096->MarkPlayer(this); 
	}
}

void ACCDCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 물건을 들고 있고 + 좌클릭 중이라면 물체를 회전
	if (InteractionComp && InteractionComp->IsRotationMode())
	{
		InteractionComp->AddRotationInput(LookAxisVector.Y, LookAxisVector.X);
	}
	else
	{
		// 평소에는 캐릭터 시선 처리
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
void ACCDCharacter::OnRotationPressed()
{
	if (HasFreezeGrenade())
	{
		FVector CamForward = FirstPersonCamera ? FirstPersonCamera->GetForwardVector() : GetActorForwardVector();
		Server_ThrowHeldGrenade(CamForward);
		return;
	}
	
	if (InteractionComp && InteractionComp->GetGrabbedComponent()) 
	{
		InteractionComp->SetRotationMode(true);
	}
}
void ACCDCharacter::OnRotationReleased()
{
	if (!InteractionComp) return;
	
	InteractionComp->SetRotationMode(false);
}

void ACCDCharacter::AddBloodToFeet(int32 StepCount)
{
	// 이미 피가 묻어있다면 횟수 누적 혹은 갱신
	RemainingFootprints = FMath::Max(RemainingFootprints, StepCount);
}
void ACCDCharacter::TrySpawnFootprint(FName FootSocketName)
{
	// UE_LOG(LogTemp, Warning, TEXT("Footstep Detected! Socket: %s, Remaining: %d"), *FootSocketName.ToString(), RemainingFootprints);
	if (GetMesh() == nullptr) return;
	
	// 소켓(혹은 본)의 월드 위치 가져오기
	FVector SocketLocation = GetMesh()->GetSocketLocation(FootSocketName);
	
	// 상태에 맞는 사운드 재생 (피가 있든 없든 소리는 나야 함)
	// RemainingFootprints가 0보다 크면 질척이는 소리, 아니면 일반 소리 선택
	USoundBase* SoundToPlay = (RemainingFootprints > 0) ? BloodyFootstepSound : NormalFootstepSound;

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, SocketLocation);
	}
	
	MakeFootstepNoise();

	// 피가 묻어있을 때만 발자국 데칼 생성 로직 수행
	if (RemainingFootprints > 0)
	{
		// 소켓 위치 기준 바닥 체크
		FHitResult Hit;
		FVector Start = SocketLocation + FVector(0.f, 0.f, 20.f); // 발등 위에서 시작
		FVector End = SocketLocation + (FVector::UpVector * -50.f);  // 바닥 아래로
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
		{
			bool bIsLeft = FootSocketName.ToString().Contains(TEXT("L"), ESearchCase::IgnoreCase);
        
			FRotator CharacterRot = GetActorRotation();
			FRotator SpawnRot = Hit.ImpactNormal.Rotation();
			SpawnRot.Pitch -= 90.0f; 
			SpawnRot.Yaw = CharacterRot.Yaw + 90.0f; 
		
			Server_SpawnFootprint(Hit.Location + Hit.ImpactNormal * 1.1f, SpawnRot, bIsLeft);
		}
	}
}
void ACCDCharacter::Server_SpawnFootprint_Implementation(FVector Location, FRotator Rotation, bool bIsLeft)
{
	if (RemainingFootprints <= 0) return;
	
	TSubclassOf<ADecal_StainActor_Base> SelectedClass = bIsLeft ? FootprintLeftDecalClass : FootprintRightDecalClass;
	if (!SelectedClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ADecal_StainActor_Base* Footprint = GetWorld()->SpawnActor<ADecal_StainActor_Base>(SelectedClass, Location, Rotation, SpawnParams))
	{
		if (UWashableComponent* WashComp = Footprint->FindComponentByClass<UWashableComponent>())
		{
			WashComp->TakeWashDamage(50.0f);
		}
		
		float Alpha = FMath::Clamp(FMath::Sqrt((float)RemainingFootprints / 6.0f), 0.3f, 1.0f);
		Footprint->UpdateDecalOpacity(Alpha);
		
		RemainingFootprints--;
	}
}

void ACCDCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	FVector LandingLocation = Hit.ImpactPoint;
	
	USoundBase* SoundToPlay = (RemainingFootprints > 0) ? BloodyLandingSound : NormalLandingSound;

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, LandingLocation);
	}
	
	float Loudness = 0.6f;
	
	if (NoiseEmitter)
	{
		NoiseEmitter->MakeNoise(this, Loudness, LandingLocation); 
	}

#if WITH_EDITOR
	if (IsLocallyControlled() && GetWorld())
	{
		float BaseHearingRange = 2500.0f;
		float SoundRadius = BaseHearingRange * Loudness;

		DrawDebugSphere(
			GetWorld(),
			LandingLocation,
			SoundRadius,
			16,
			FColor::Magenta,
			false,
			0.8f,
			0,
			2.5f
		);
	}
#endif
}

void ACCDCharacter::MakeFootstepNoise(float LoudnessMultiplier)
{
	if (!NoiseEmitter) return;

	// 기본 소음 크기
	float FinalLoudness = 0.3f * LoudnessMultiplier;
	
	// 달리기 소음
	if (StatComp && StatComp->GetIsRunning())
	{
		FinalLoudness = 0.6f;
	}
	/*
	else if (GetCharacterMovement() && GetCharacterMovement()->IsCrouching())
	{
		FinalLoudness = 0.0f; // 앉아서 걸을 때는 SCP-939가 못 듣도록 기획했다면 0으로 처리
	}
	*/
	
	if (FinalLoudness > 0.0f)
	{
		NoiseEmitter->MakeNoise(this, FinalLoudness, GetActorLocation());
		
#if WITH_EDITOR
		if (IsLocallyControlled() && GetWorld())
		{
			float BaseHearingRange = 2500.0f;
			float SoundRadius = BaseHearingRange * FinalLoudness;
			
			DrawDebugSphere(
				GetWorld(),
				GetActorLocation(),
				SoundRadius,
				16,
				FColor::Red,
				false,
				1.0f, 
				0,
				2.0f  
			);
		}
#endif
	}
	
	
}

float ACCDCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	if (!HasAuthority() || ActualDamage <= 0.f || bIsDead || bIsInvincible) 
	{
		return 0.f;
	}
	
	CurrentHealth -= ActualDamage;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	
	if (HurtSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			HurtSound, 
			GetActorLocation(), 
			1.0f, // 볼륨 배율
			1.0f  // 피치 배율
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("Took Damage. Current HP: %f"), CurrentHealth);
	
	if (IsLocallyControlled())
	{
		OnRep_CurrentHealth();
	}
	
	if (CurrentHealth <= 0.f)
	{
		Server_Die();
	}

	return ActualDamage;
}

void ACCDCharacter::OnRep_CurrentHealth()
{
	if (IsLocallyControlled())
	{
		if (DamageWidgetClass && !DamageWidgetInstance)
		{
			DamageWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), DamageWidgetClass);
			if (DamageWidgetInstance)
			{
				DamageWidgetInstance->AddToViewport();
			}
		}
		
		if (CurrentHealth >= MaxHealth)
		{
			return; 
		}
		
		float Ratio = MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
		OnDamageEffectTriggered(Ratio);
	}
}

void ACCDCharacter::Server_ThrowHeldGrenade_Implementation(FVector LaunchDir)
{
	if (!HasAuthority() || !HeldGrenade) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[Character] Throw aborted. HeldGrenade is NULL."));
		return;
	}
	
	ACCD_FreezeGrenade* GrenadeToThrow = HeldGrenade;
	HeldGrenade = nullptr;

	if (InteractionComp && InteractionComp->GetGrabbedComponent())
	{
		InteractionComp->ForceRelease(); 
	}
	
	if (GrenadeToThrow)
	{
		GrenadeToThrow->Launch(LaunchDir);
	}
}