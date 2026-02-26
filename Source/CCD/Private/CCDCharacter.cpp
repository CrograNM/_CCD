
#include "CCDCharacter.h"

#include "CCDPlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Component/CCD_EquipmentComponent.h"
#include "Component/CCD_InteractionComponent.h"
#include "Component/CCD_ViewComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

/** --- 생성자 및 기본 함수 --- */
ACCDCharacter::ACCDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// --- 기능성 컴포넌트 추가 ---
	ViewComp = CreateDefaultSubobject<UCCD_ViewComponent>(TEXT("ViewComp"));
	InteractionComp = CreateDefaultSubobject<UCCD_InteractionComponent>(TEXT("InteractionComp"));
	EquipmentComp = CreateDefaultSubobject<UCCD_EquipmentComponent>(TEXT("EquipmentComp"));
	
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
}
void ACCDCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACCDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

/** --- 사망 처리 --- */
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

/** --- 이동 속도 변경 --- */
void ACCDCharacter::Server_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}
void ACCDCharacter::SetRunning(float NewSpeed)
{
	// 1. 로컬 속도를 즉시 변경 (클라이언트 예측을 위해)
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

	// 2. 서버에게도 속도 변경을 요청
	Server_SetMaxWalkSpeed(NewSpeed);
}

void ACCDCharacter::OnRep_IsDead()
{
	// 캐릭터 메쉬 숨기기
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
	}

	// 3인칭 시점으로 강제 전환 및 고정
	if (ViewComp)
	{
		ViewComp->ApplyViewMode(false); // false는 3인칭 (FollowCamera 활성화)
	}

	// 화면 어둡게 처리 (사망한 로컬 플레이어)
	if (IsLocallyControlled())
	{
		if (ACCDPlayerController* PC = Cast<ACCDPlayerController>(GetController()))
		{
			PC->ApplyDeathOverlay(true);
		}
	}
}

void ACCDCharacter::HandleDeath()
{
	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 물리 상호작용 정리 (잡고 있던 물체 투하)
	if (InteractionComp) InteractionComp->ForceRelease();
	
	// 장비 정리 (장비 액터 파괴)
	if (EquipmentComp)EquipmentComp->DestroyAllEquipment();
	
	// 이동 능력 상실
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
	
	// ------------------------------------
	// 추후 카오스 디스트럭션 적용 예정
	// ------------------------------------
	
}
