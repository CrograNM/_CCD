// Fill out your copyright notice in the Description page of Project Settings.


#include "CCDCharacter.h"
#include "InteractInterface.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACCDCharacter::ACCDCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 3인칭 카메라
	// - SpringArm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); // 캐릭터(캡슐)에 부착
	CameraBoom->TargetArmLength = 400.0f;       // 캐릭터와의 거리
	CameraBoom->bUsePawnControlRotation = true; // 컨트롤러(마우스)에 따라 회전하도록 설정

	// - Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // SpringArm 끝에 부착
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 암에 붙어있으므로 직접 회전할 필요 없음
	
	// 1인칭 카메라
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("HeadSocket")); // 캐릭터 스켈레톤의 'head' 소켓에 부착
	FirstPersonCamera->bUsePawnControlRotation = true; // 마우스 회전에 따라 카메라 회전
	FirstPersonCamera->SetAutoActivate(false); // 시작할 때는 꺼둠
	
	// 장비 메쉬 생성 및 부착
	// 1. 대걸레 생성 및 부착
	MopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MopMesh"));
	MopMesh->SetIsReplicated(true); // 컴포넌트 복제 활성화
	MopMesh->SetCollisionResponseToAllChannels(ECR_Ignore); // 충돌 방지
    
	// 캐릭터의 Mesh(Skeletal Mesh)에 있는 'MopSocket'에 부착
	MopMesh->SetupAttachment(GetMesh(), TEXT("MopSocket_Back"));

	// 2. 탐지장치 생성 및 부착
	ScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScannerMesh"));
	ScannerMesh->SetIsReplicated(true); // 컴포넌트 복제 활성화
	ScannerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    
	// 캐릭터의 Mesh에 있는 'ScannerSocket'에 부착
	ScannerMesh->SetupAttachment(GetMesh(), TEXT("ScannerSocket_Hip"));
}

// Called when the game starts or when spawned
void ACCDCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 복제할 변수 등록
void ACCDCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDCharacter, EquipmentState); // 변수 복제 등록
}

// 서버 RPC 구현
void ACCDCharacter::Server_SetEquipmentState_Implementation(ECCD_EquipmentState NewState)
{
	// 이미 같은 상태이거나, 현재 장비를 넣는(Unequip) 중이라면 무시합니다.
	if (EquipmentState == NewState || bIsUnequipping) return;

	// 1. 다음에 바꿀 상태를 미리 저장해둡니다.
	PendingEquipmentState = NewState;

	// 2. 기존 장비가 있다면 역재생(Unequip) 시작
	if (EquipmentState != ECCD_EquipmentState::EES_Hands)
	{
		bIsUnequipping = true;
		FName SectionName = (EquipmentState == ECCD_EquipmentState::EES_Mop) ? TEXT("DrawMop") : TEXT("DrawScanner");
        
		// 멀티캐스트로 모든 유저에게 역재생 명령
		Multicast_PlayEquipMontage(SectionName, -1.2f);
		
		BindMontageEndedDelegate();
	}
	else
	{
		// 맨손 상태였다면 즉시 새 장비 장착 시작
		ProceedToEquip(NewState);
	}
}

void ACCDCharacter::ProceedToEquip(ECCD_EquipmentState NewState)
{
	bIsUnequipping = false;

	// 최종 목표가 맨손이면 추가 재생 없이 종료
	if (NewState == ECCD_EquipmentState::EES_Hands)
	{
		Multicast_StopMontage();
		return;
	}
    
	// 새 장비 꺼내기 애니메이션 재생
	FName SectionName = (NewState == ECCD_EquipmentState::EES_Mop) ? TEXT("DrawMop") : TEXT("DrawScanner");
	Multicast_PlayEquipMontage(SectionName, 1.0f);
}

// 클라이언트 통지 구현
void ACCDCharacter::OnRep_EquipmentState(ECCD_EquipmentState PreviousState)
{
	HandleEquipmentEffects(EquipmentState);
}

// 실제 시각적/기능적 스위칭 로직
void ACCDCharacter::HandleEquipmentEffects(ECCD_EquipmentState NewState)
{
	// 가시성 초기화
	MopMesh->SetHiddenInGame(false);
	ScannerMesh->SetHiddenInGame(false);

	// 애니메이션 인스턴스를 가져와 현재 몽타주가 재생 중인지 확인
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	bool bIsPlayingEquipAnim = AnimInstance && AnimInstance->Montage_IsPlaying(EquipMontage);

	// 만약 몽타주가 재생 중이라면, 소켓 이동은 노티파이가 처리할 것.
	if (bIsPlayingEquipAnim)
	{
		UE_LOG(LogTemp, Log, TEXT("HandleEquipmentEffects: Montage is playing. Yielding to Notify."));
		return;
	}

	// 3. 몽타주가 재생 중이 아닐 때만(예: 중도 참가자, 애니메이션 종료 후) 소켓을 확정 짓습니다.
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

// 몽타주를 즉시 멈추기 위한 멀티캐스트 추가
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
	// 서버에서만 상태 전환을 처리합니다.
	if (!HasAuthority()) return;

	// 현재 역재생(Unequip) 중이었다면, 이제 실제 장착을 진행합니다.
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
        
		// 몽타주가 끝날 때(또는 중단될 때) 호출되도록 설정
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, EquipMontage);
	}
}

void ACCDCharacter::ToggleView()
{
	bIsFirstPerson = !bIsFirstPerson;

	if (bIsFirstPerson)
	{
		// 1인칭 시점으로 전환
		FollowCamera->SetActive(false);
		FirstPersonCamera->SetActive(true);
		GetMesh()->SetOwnerNoSee(true);
	}
	else
	{
		// 3인칭 시점으로 전환
		FirstPersonCamera->SetActive(false);
		FollowCamera->SetActive(true);
		GetMesh()->SetOwnerNoSee(false);
	}
}

void ACCDCharacter::TestCurrentState()
{
	// 1. 현재 장비 상태를 문자열로 변환
	FString StateString;
	switch (EquipmentState)
	{
	case ECCD_EquipmentState::EES_Hands:   StateString = TEXT("Hands (Physics Handle)"); break;
	case ECCD_EquipmentState::EES_Scanner: StateString = TEXT("Scanner"); break;
	case ECCD_EquipmentState::EES_Mop:     StateString = TEXT("Mop"); break;
	}
	// 2. 권한 및 로컬 역할 확인 
	FString AuthoritySide = HasAuthority() ? TEXT("Server") : TEXT("Client");
    
	// 3. 최종 메시지 구성
	FString FinalMessage = FString::Printf(TEXT("[%s] Current State: %s"), *AuthoritySide, *StateString);

	// 출력 로그(Output Log)에 출력
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalMessage);
}

void ACCDCharacter::HandleEquipNotify()
{
	// [시각적 처리] 서버와 클라이언트 모두에서 실행 (애니메이션과 위치 동기화)
	if (bIsUnequipping)
	{
		// 역재생 중 노티파이: 보관 위치로 이동
		MopMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
        
		// [서버 전용] 물체를 집어넣었으므로 이제 '맨손' 상태로 변경
		if (HasAuthority())
		{
			EquipmentState = ECCD_EquipmentState::EES_Hands;
		}
	}
	else
	{
		// 정재생 중 노티파이: 손으로 이동
		if (PendingEquipmentState == ECCD_EquipmentState::EES_Mop)
		{
			MopMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Hand"));
		}
		else if (PendingEquipmentState == ECCD_EquipmentState::EES_Scanner)
		{
			ScannerMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hand"));
		}

		// [서버 전용] 이제 새로운 장비를 들었으므로 상태 업데이트
		if (HasAuthority())
		{
			EquipmentState = PendingEquipmentState;
		}
	}
}

void ACCDCharacter::PerformInteract()
{
	// 1. 변수가 유효한지(nullptr 체크) 먼저 확인합니다.
	if (FirstPersonCamera == nullptr) return;

	// 2. 변수명을 직접 사용하여 위치와 방향을 가져옵니다.
	FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	FVector TraceEnd = TraceStart + (FirstPersonCamera->GetForwardVector() * InteractRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// 라인트레이스 실행
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Interacted with: %s"), *HitActor->GetName());
			// 인터페이스 캐스팅 (I를 하나만 쓰는 이름으로 수정했다고 가정)
			IInteractInterface* Interface = Cast<IInteractInterface>(HitActor);
			if (Interface)
			{
				UE_LOG(LogTemp, Warning, TEXT("Interface found on: %s"), *HitActor->GetName());
				Interface->Interact(this);
			}
		}
	}
}


