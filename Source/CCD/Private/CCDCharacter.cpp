// Fill out your copyright notice in the Description page of Project Settings.


#include "CCDCharacter.h"

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
