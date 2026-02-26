
#include "CCDCharacter.h"

#include "CCDPlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Component/CCD_EquipmentComponent.h"
#include "Component/CCD_InteractionComponent.h"
#include "Component/CCD_ViewComponent.h"
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

void ACCDCharacter::PerformInteract()
{
	// 이제 모든 복잡한 트레이스/잡기 로직은 컴포넌트가 알아서 합니다.
	if (InteractionComp)
	{
		InteractionComp->PerformInteract();
	}
}

/** --- 장비 및 뷰 모드 --- */
void ACCDCharacter::SwitchEquipment(const ECCD_EquipmentState NewState)
{
	if (EquipmentComp) EquipmentComp->SwitchEquipment(NewState);
}
void ACCDCharacter::ToggleView()
{
	if (ViewComp)
	{
		ViewComp->ToggleView(); // 복잡한 동기화는 컴포넌트가 처리
	}
}
void ACCDCharacter::UseEquipment()
{
	Server_UseEquipment();
}

void ACCDCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("[ACCDCharacter] Die called"));
}

void ACCDCharacter::Server_UseEquipment_Implementation()
{
	if (EquipmentComp)
	{
		EquipmentComp->ExecuteActiveEquipment();
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
void ACCDCharacter::SetRunning(float NewSpeed)
{
	// 1. 로컬 속도를 즉시 변경 (클라이언트 예측을 위해)
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

	// 2. 서버에게도 속도 변경을 요청
	Server_SetMaxWalkSpeed(NewSpeed);
}
void ACCDCharacter::Server_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}