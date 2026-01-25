
#include "CCDCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interface/InteractInterface.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Net/UnrealNetwork.h"

/** --- 생성자 및 기본 함수 --- */
ACCDCharacter::ACCDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// --- 카메라 설정 ---
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("HeadSocket"));
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	// 카메라 초기 상태 설정	( 3인칭 모드 시작 )
	FollowCamera->SetActive(!bIsFirstPerson);		// 카메라 활성화 상태 변경
	FirstPersonCamera->SetActive(bIsFirstPerson);	
	GetMesh()->SetOwnerNoSee(bIsFirstPerson);		// 메시 가시성 처리
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	// --- 장비 메시 설정 ---
	MopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MopMesh"));
	MopMesh->SetIsReplicated(true);
	MopMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	MopMesh->SetupAttachment(GetMesh(), TEXT("MopSocket_Back"));

	ScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScannerMesh"));
	ScannerMesh->SetIsReplicated(true);
	ScannerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ScannerMesh->SetupAttachment(GetMesh(), TEXT("ScannerSocket_Hip"));
	
	// --- 물리 핸들 ---
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}
void ACCDCharacter::BeginPlay()
{
	Super::BeginPlay();
}
void ACCDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/** --- 입력 및 상호작용 --- */
void ACCDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void ACCDCharacter::ToggleView()
{
	bIsFirstPerson = !bIsFirstPerson;
	FollowCamera->SetActive(!bIsFirstPerson);		// 카메라 활성화 상태 변경
	FirstPersonCamera->SetActive(bIsFirstPerson);	
	GetMesh()->SetOwnerNoSee(bIsFirstPerson);		// 메시 가시성 처리

	// 이동 및 회전 로직 변경
	if (bIsFirstPerson)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}
void ACCDCharacter::PerformInteract()
{
	if (!FirstPersonCamera) return;

	FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	FVector TraceEnd = TraceStart + (FirstPersonCamera->GetForwardVector() * InteractRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		if (HitResult.GetActor())
		{
			AActor* HitActor = HitResult.GetActor();
			UActorComponent* InteractableComp = HitActor->FindComponentByInterface(UInteractInterface::StaticClass());

			if (InteractableComp)
			{
				// 컴포넌트의 인터페이스 함수 실행
				IInteractInterface::Execute_Interact(InteractableComp, this);
			}
		}
	}
}

void ACCDCharacter::Server_PlayActionOfState_Implementation()
{
	if (bIsActionInProgress) return;
	
	// 실제 재생 여부 더블 체크
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(EquipMontage)) return;
	
	// 1. 현재 장비 상태를 문자열로 변환
	FString StateString;
	switch (EquipmentState)
	{
	case ECCD_EquipmentState::EES_Hands:   
		StateString = TEXT("Hands (Physics Handle)"); 
		break;
	case ECCD_EquipmentState::EES_Mop:     
		StateString = TEXT("Mop"); 
		bIsActionInProgress = true;	// 액션 진행중 플래그 설정
		Multicast_PlayEquipMontage(TEXT("SwingMop"), 1.5f);	// 대걸레 휘두르기 애니메이션 재생
		BindMontageEndedDelegate();	// 몽타주 종료 델리게이트 바인딩
		break;
	case ECCD_EquipmentState::EES_Scanner: 
		StateString = TEXT("Scanner"); 
		break;
	}
	// 2. 권한 및 로컬 역할 확인 
	FString AuthoritySide = HasAuthority() ? TEXT("Server") : TEXT("Client");
    
	// 3. 최종 메시지 구성
	FString FinalMessage = FString::Printf(TEXT("[%s] Current State: %s"), *AuthoritySide, *StateString);

	// 출력 로그(Output Log)에 출력
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalMessage);
}

/** --- 장비 관리 시스템 (네트워크) --- */
void ACCDCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 변수 복제 등록
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDCharacter, EquipmentState);
}
void ACCDCharacter::Server_SetEquipmentState_Implementation(ECCD_EquipmentState NewState)
{
	if (EquipmentState == NewState || bIsUnequipping || bIsActionInProgress) return;

	PendingEquipmentState = NewState;

	if (EquipmentState != ECCD_EquipmentState::EES_Hands)
	{
		bIsUnequipping = true;
		FName Section = (EquipmentState == ECCD_EquipmentState::EES_Mop) ? TEXT("DrawMop") : TEXT("DrawScanner");
		bIsActionInProgress = true;	
		Multicast_PlayEquipMontage(Section, -1.2f);
		BindMontageEndedDelegate();
	}
	else
	{
		ProceedToEquip(NewState);
	}
}
void ACCDCharacter::ProceedToEquip(ECCD_EquipmentState NewState)
{
	bIsUnequipping = false;

	if (NewState == ECCD_EquipmentState::EES_Hands)
	{
		Multicast_StopMontage();
		HandleEquipmentEffects(NewState);
		return;
	}

	FName Section = (NewState == ECCD_EquipmentState::EES_Mop) ? TEXT("DrawMop") : TEXT("DrawScanner");
	bIsActionInProgress = true;	
	Multicast_PlayEquipMontage(Section, 1.0f);
	BindMontageEndedDelegate();
	HandleEquipmentEffects(NewState);
}
void ACCDCharacter::OnRep_EquipmentState(ECCD_EquipmentState PreviousState)
{
	HandleEquipmentEffects(EquipmentState);
}

/** --- 시각 효과 및 애니메이션 --- */
void ACCDCharacter::HandleEquipmentEffects(ECCD_EquipmentState NewState)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(EquipMontage)) return;
	
	// 비-재생 중(중도 참가자 등)일 때의 최종 소켓 확정
	switch (NewState)
	{
	case ECCD_EquipmentState::EES_Hands:
		MopMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		break;

	case ECCD_EquipmentState::EES_Scanner:
		ScannerMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hand"));
		MopMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		break;

	case ECCD_EquipmentState::EES_Mop:
		MopMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Hand"));
		ScannerMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		break;
	}
}
void ACCDCharacter::HandleEquipNotify()
{
	if (!HasAuthority()) return;

	if (bIsUnequipping)
	{
		MopMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		EquipmentState = ECCD_EquipmentState::EES_Hands;
	}
	else
	{
		FName Socket = (PendingEquipmentState == ECCD_EquipmentState::EES_Mop) ? TEXT("MopSocket_Hand") : TEXT("ScannerSocket_Hand");
		UStaticMeshComponent* TargetMesh = (PendingEquipmentState == ECCD_EquipmentState::EES_Mop) ? MopMesh : ScannerMesh;
		TargetMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
		EquipmentState = PendingEquipmentState;
	}
}

/** --- 몽타주 제어 및 델리게이트 --- */
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
		ProceedToEquip(PendingEquipmentState);
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

