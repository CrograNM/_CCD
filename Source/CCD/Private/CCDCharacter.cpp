
#include "CCDCharacter.h"
#include "Camera/CameraComponent.h"
#include "Component/BurnableComponent.h"
#include "Component/WashableComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
	
	// --- FirstPersonCamera 회전 동기화 ---
	if (IsLocallyControlled())
	{
		// 현재 로컬 카메라 회전값 가져오기
		FRotator CurrentRot = FirstPersonCamera->GetRelativeRotation();

		// 임계값 비교: 마지막 전송값과 차이가 날 때만 서버에 전송
		if (!CurrentRot.Equals(LastSentRotation, RotationThreshold))
		{
			Server_SetFirstPersonCameraRotation(CurrentRot);
			Server_SetControlRotation(GetControlRotation());
			LastSentRotation = CurrentRot;
		}
	}
	else
	{
		FRotator NewRot = FMath::RInterpTo(FirstPersonCamera->GetRelativeRotation(), Rep_FirstPersonCameraRotation, DeltaTime, 15.0f);
		FirstPersonCamera->SetRelativeRotation(NewRot);
	}
	
	// --- 잡고 있는 물체 위치 업데이트 ---
	UpdatePhysicsHandleTarget();
}

void ACCDCharacter::UpdatePhysicsHandleTarget()
{
	if (HasAuthority() && PhysicsHandle && GrabbedComponent)
	{
		FVector LookDir = FirstPersonCamera->GetForwardVector();
		FVector TargetLocation = FirstPersonCamera->GetComponentLocation() + (LookDir * 200.0f);
		PhysicsHandle->SetTargetLocation(TargetLocation);
	}
}


void ACCDCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 변수 복제 등록
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDCharacter, bIsFirstPerson);
	DOREPLIFETIME(ACCDCharacter, EquipmentState);
	DOREPLIFETIME(ACCDCharacter, Rep_FirstPersonCameraRotation); // 회전값 복제 -> FirstPersonCamera에 적용시킴
	DOREPLIFETIME(ACCDCharacter, RemoteControlRotation);
	DOREPLIFETIME(ACCDCharacter, GrabbedComponent);
}

/** --- 입력 및 상호작용 --- */
void ACCDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACCDCharacter::ToggleView()
{
	// 1. 로컬에서 즉시 변경
	bIsFirstPerson = !bIsFirstPerson;
	ApplyViewMode(bIsFirstPerson);
	
	// 2. 서버에도 알림
	Server_ToggleView(bIsFirstPerson);
}
void ACCDCharacter::Server_ToggleView_Implementation(bool bNewIsFirstPerson)
{
	bIsFirstPerson = bNewIsFirstPerson;
	ApplyViewMode(bIsFirstPerson);
}
void ACCDCharacter::ApplyViewMode(bool bFirstPerson)
{
	FollowCamera->SetActive(!bFirstPerson);
	FirstPersonCamera->SetActive(bFirstPerson);
	GetMesh()->SetOwnerNoSee(bFirstPerson);

	if (bFirstPerson)
	{
		bUseControllerRotationYaw = true; // 이제 서버에서도 true가 된다.
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
	Server_PerformInteract();
}

void ACCDCharacter::Server_PerformInteract_Implementation()
{
	// 이미 물체를 잡고 있다면 놓기
	if (GrabbedComponent)
	{
		Server_ReleaseObject();
		return;
	}
	// 맨손 상태가 아니면 상호작용 불가
	if (EquipmentState != ECCD_EquipmentState::EES_Hands) return;

	FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	FVector TraceEnd = TraceStart + (FirstPersonCamera->GetForwardVector() * InteractRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// 라인트레이스로 컴포넌트 확인
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) return;
		
		// 피직스 핸들 발동 : 히트된 액터에서 BurnableComponent를 찾습니다.
		if (UBurnableComponent* BurnComp = HitActor->FindComponentByClass<UBurnableComponent>())
		{
			// physics simulate 중인 메쉬인지 확인
			UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
			if (RootPrim && RootPrim->IsSimulatingPhysics())
			{
				Server_GrabObject(RootPrim, NAME_None, HitResult.ImpactPoint);
			}
			IInteractInterface::Execute_Interact(BurnComp, this);
		}
		// 이외의 상호작용 가능 액터들에 대해서도 인터페이스 호출
		else if (HitActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("ACCDCharacter::PerformInteract Interacted with %s"), *HitActor->GetName());
			IInteractInterface::Execute_Interact(HitActor, this);
		}
	}
}

void ACCDCharacter::Server_GrabObject_Implementation(UPrimitiveComponent* ComponentToGrab, FName BoneName, FVector GrabLocation)
{
	if (!PhysicsHandle || !ComponentToGrab) return;

	// 서버에서 변수 할당 (이 값이 클라이언트에게 복제됨)
	GrabbedComponent = ComponentToGrab;
	
	GrabbedComponent->SetSimulatePhysics(false);
	GrabbedComponent->IgnoreActorWhenMoving(this, true);
	
	// 물리 핸들로 잡기 실행
	PhysicsHandle->GrabComponentAtLocationWithRotation(
		ComponentToGrab,
		BoneName,
		GrabLocation,
		ComponentToGrab->GetComponentRotation()
	);
}
void ACCDCharacter::Server_ReleaseObject_Implementation()
{
	if (PhysicsHandle && GrabbedComponent)
	{
		GrabbedComponent->SetSimulatePhysics(true);
		GrabbedComponent->IgnoreActorWhenMoving(this, false);
		
		PhysicsHandle->ReleaseComponent();
	}
	
	// 서버에서 변수 해제 (클라이언트도 nullptr로 바뀜)
	GrabbedComponent = nullptr;
}
void ACCDCharacter::Server_PlayActionOfMop_Implementation()
{
	if (bIsActionInProgress) return;
	
	// 실제 재생 여부 더블 체크
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(EquipMontage)) return;
	
	if (EquipmentState == ECCD_EquipmentState::EES_Mop)
	{
		PerformCleaningTrace();
		
		bIsActionInProgress = true;	// 액션 진행중 플래그 설정
		Multicast_PlayEquipMontage(TEXT("SwingMop"), 1.5f);	// 대걸레 휘두르기 애니메이션 재생
		BindMontageEndedDelegate();	// 몽타주 종료 델리게이트 바인딩
	}
}

/** --- 장비 관리 시스템 (네트워크) --- */
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

void ACCDCharacter::PerformCleaningTrace() 
{
	UE_LOG( LogTemp, Warning, TEXT("PerformCleaningTrace called") );
	
	if (!HasAuthority()) return; // 세척 판정은 서버에서만 수행
	if (EquipmentState != ECCD_EquipmentState::EES_Mop) return;

	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + (FirstPersonCamera->GetForwardVector() * InteractRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) return;
		
		// 히트된 액터에서 WashableComponent를 찾습니다.
		if (UWashableComponent* WashComp = HitResult.GetActor()->FindComponentByClass<UWashableComponent>())
		{
			UE_LOG(LogTemp, Warning, TEXT("WashComp %s Interacted"), *HitActor->GetName());
			WashComp->TakeWashDamage(25.f); // 예시로 25의 세척 데미지 적용
		}
	}
}

void ACCDCharacter::Server_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ACCDCharacter::Server_SetFirstPersonCameraRotation_Implementation(FRotator NewRotation)
{
	Rep_FirstPersonCameraRotation = NewRotation;
}

void ACCDCharacter::Server_SetControlRotation_Implementation(FRotator NewRotation)
{
	RemoteControlRotation = NewRotation;
}


