// Fill out your copyright notice in the Description page of Project Settings.


#include "CCDCharacter.h"
#include "InteractInterface.h"
#include "Camera/CameraComponent.h"

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
	MopMesh->SetupAttachment(GetMesh(), TEXT("MopSocket"));

	// 2. 탐지장치 생성 및 부착
	ScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScannerMesh"));
	ScannerMesh->SetIsReplicated(true); // 컴포넌트 복제 활성화
	ScannerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    
	// 캐릭터의 Mesh에 있는 'ScannerSocket'에 부착
	ScannerMesh->SetupAttachment(GetMesh(), TEXT("ScannerSocket"));
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