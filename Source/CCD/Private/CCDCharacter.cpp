
#include "CCDCharacter.h"

#include "CCDPlayerController.h"
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
#include "GameFramework/GameModeBase.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Net/UnrealNetwork.h"

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
}
void ACCDCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACCDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 살아있을 때만 시선 체크 수행
	if (!bIsDead)
	{
		CheckForSCP096();
	}
}

void ACCDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
void ACCDCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 변수 복제 등록
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDCharacter, RemoteControlRotation);
}

/** --- 입력 바인딩 : 상호작용, 1-3인칭 전환, 장비 전환 및 사용 --- */
void ACCDCharacter::PerformInteract()
{
	if (bIsDead) return;
	if (EquipmentComp->GetEquipmentState() != ECCD_EquipmentState::EES_Hands) return;
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

/** --- 사망 및 부활 처리 --- */
void ACCDCharacter::Die()
{	
	Server_Die();
}
void ACCDCharacter::Server_Die_Implementation()
{	
	if (bIsDead) return; // 이미 사망한 경우 중복 처리 방지
	UE_LOG(LogTemp, Warning, TEXT("[ACCDCharacter] Die called"));
	
	bIsDead = true;
	// 서버 전용 (충돌 비활성, 물체 투하, 장비 제거, 이동 불가), 이후 카오스 디스트럭션 적용 예정
	HandleDeath();	
	// 클라이언트 전용 (로컬 시각 효과), 서버도 명시적으로 적용
	OnRep_IsDead(); 
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
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Stop(0.2f, EquipMontage);
	}
}
void ACCDCharacter::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!HasAuthority()) return;
	
	bIsActionInProgress = false; // 액션 상태 해제
	
	// 만약 장비 교체 중이었다면 기존 로직 수행
	if (bIsUnequipping)
	{
		EquipmentComp->ProceedToEquip(EquipmentComp->GetPendingEquipmentState());
	}
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
			RootPrim->SetCollisionProfileName(TEXT("Pawn"));
		if (ViewComp) ViewComp->ApplyViewMode(ViewComp->GetIsFirstPerson());
	}
	
	// 사망자 본인 처리
	if (IsLocallyControlled())
	{
		if (ACCDPlayerController* PC = Cast<ACCDPlayerController>(GetController()))
		{
			if (!bIsDead) PC->ResetPlayerController(this);
			else PC->ApplyDeath(bIsDead);
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
		for (int i = 0; i < 3; ++i)
		{
			FHitResult HitResult;
			FVector Start = GetActorLocation() + FVector(0.f, 0.f, BloodSpawnHeight);
			FVector End = Start + FVector(FMath::RandRange(-BloodSpawnRange, BloodSpawnRange), FMath::RandRange(-BloodSpawnRange, BloodSpawnRange), -500.0f - BloodSpawnHeight);
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
	
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
				// 바닥 평면에 맞춘 회전값 (바닥 노멀 기준)
				FRotator SpawnRot = HitResult.ImpactNormal.Rotation();
				SpawnRot.Pitch -= 90.0f; // 데칼은 기본적으로 X축 방향으로 쏘므로 아래를 향하게 조정
		
				if (ADecal_StainActor_Base* SpawnedDecal = GetWorld()->SpawnActor<ADecal_StainActor_Base>(
					BloodStainActorClass, 
					HitResult.Location + FVector(0.f, 0.f, FMath::RandRange(-0.1f, 0.1f)), 
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
		for (USkeletalMesh* FragmentMesh : FragmentMeshList)
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
				// // 랜덤한 방향으로 튀어나가게 충격 가하기
				// FVector RandomImpulse = (FVector::UpVector + FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f)).GetSafeNormal() * DeathImpulseStrength;
				// Fragment->InitFragment(FragmentMesh, RandomImpulse);
				
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
}

// SCP 상호작용
void ACCDCharacter::CheckForSCP096()
{
	// 로컬 플레이어가 조정 중일 때만 시선 체크를 수행
	if (!IsLocallyControlled()) return;

	// 현재 사용 중인 카메라 컴포넌트 가져오기 (1인칭 기준)
	if (!FirstPersonCamera) return;

	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector ForwardVector = FirstPersonCamera->GetForwardVector();
	FVector End = Start + (ForwardVector * 5000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 나 자신은 무시

	// Line Trace 실행
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// 맞은 액터가 SCP-096인지 확인
		if (ACCD_096* SCP096 = Cast<ACCD_096>(Hit.GetActor()))
		{
			// 맞은 컴포넌트가 096의 얼굴 트리거인지 확인
			if (Hit.GetComponent() == SCP096->GetFaceTrigger())
			{
				// 이미 화가 난 상태가 아니라면 트리거 발동
				if (!SCP096->IsTriggered())
				{
					SCP096->TriggerPanic(this); // 격노 시작
				}
			}
		}
	}
}