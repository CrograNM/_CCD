
#include "CCDCharacter.h"

#include "Actor/WaterBucketActor.h"
#include "Camera/CameraComponent.h"
#include "Component/WashableComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Component/CCD_EquipmentComponent.h"
#include "Component/CCD_InteractionComponent.h"
#include "Component/CCD_ScannerComponent.h"
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
	
	// --- 장비 메시 설정 ---
	MopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MopMesh"));
	MopMesh->SetIsReplicated(true);
	MopMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	MopMesh->SetupAttachment(GetMesh(), TEXT("MopSocket_Back"));
	
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
	
	// 대걸레 머티리얼 인스턴스 생성
	if (MopMesh)
	{
		MopMaterial = MopMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
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

/** --- 장비 전환 및 뷰 모드 --- */ // 장비 로직 분리하기
void ACCDCharacter::SwitchToHands()
{
	if (EquipmentComp) EquipmentComp->SwitchEquipment(ECCD_EquipmentState::EES_Hands);
}
void ACCDCharacter::SwitchToMop()
{
	if (EquipmentComp) EquipmentComp->SwitchEquipment(ECCD_EquipmentState::EES_Mop);
}
void ACCDCharacter::SwitchToScanner()
{
	if (EquipmentComp) EquipmentComp->SwitchEquipment(ECCD_EquipmentState::EES_Scanner);
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
	if (EquipmentComp)
	{
		EquipmentComp->ExcuteActiveEquipment();
	}
}

void ACCDCharacter::PerformCleaningTrace() const
{
	if (!HasAuthority()) return; // 세척 판정은 서버에서만 수행
	if (EquipmentComp->GetEquipmentState() != ECCD_EquipmentState::EES_Mop) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[Mop] Pollution - Blood: %f, Excrement: %f"), 
		EquipmentComp->MopPollution_Blood, EquipmentComp->MopPollution_Excrement);

	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + (FirstPersonCamera->GetForwardVector() * InteractRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) return;
		UE_LOG(LogTemp, Warning, TEXT("[Mop] Interact with : %s"), *HitActor->GetName());
		
		// 물양동이
		if (AWaterBucketActor* Bucket = Cast<AWaterBucketActor>(HitActor))
		{
			if (!Bucket->IsWaterSpilled() && Bucket->WashMop(EquipmentComp->MopPollution_Blood, EquipmentComp->MopPollution_Excrement))
			{
				EquipmentComp->UpdateMopMeshPollution();
			}
			return;
		}
		if ( EquipmentComp->MopPollution_Blood + EquipmentComp->MopPollution_Excrement >= 1.0f )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Mop] Mop is too dirty to clean."));
			
			// 데칼 생성 로직 추가하기
			
			return;
		}
		// 데칼
		if (UWashableComponent* WashComp = HitResult.GetActor()->FindComponentByClass<UWashableComponent>())
		{
			WashComp->TakeWashDamage(25.f); // 예시로 25의 세척 데미지 적용
			if (WashComp->GetWashableType() == ECCD_WashableType::EWT_Blood)
			{
				EquipmentComp->MopPollution_Blood += 0.2f; // 피 오염도 증가
			}
			else if (WashComp->GetWashableType() == ECCD_WashableType::EWT_Excrement)
			{
				EquipmentComp->MopPollution_Excrement += 0.2f; // 배설물 오염도 증가
			}
			EquipmentComp->UpdateMopMeshPollution();
		}
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
		PerformCleaningTrace();
		
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