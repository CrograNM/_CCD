// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_173.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACCD_173::ACCD_173()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ObservationSocketNames.Add(FName("socket_head"));
	
	ScreamAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ScreamAudio"));
	ScreamAudio->SetupAttachment(GetRootComponent());
	ScreamAudio->bAutoActivate = false;
	
	MoveAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("MoveAudio"));
	MoveAudio->SetupAttachment(GetRootComponent());
	MoveAudio->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ACCD_173::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCD_173::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCD_173::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ACCD_173::IsObserved()
{
	// 1. 플레이어 카메라 정보 가져오기
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!CameraManager) return false;

	FVector CameraLoc = CameraManager->GetCameraLocation();
	FVector CameraForward = CameraManager->GetCameraRotation().Vector();

	// 2. 시야각(FOV) 체크: 내적(Dot Product) 사용
	FVector ToMe = GetActorLocation() - CameraLoc;
	ToMe.Normalize();

	float Dot = FVector::DotProduct(CameraForward, ToMe);

	// 시야 밖이면 더 계산할 것도 없이 false
	if (Dot < VisibilityThreshold) return false;

	// 3. 가려짐 체크: 소켓 위치로 라인 트레이스
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 나 자신은 무시
	Params.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)); // 플레이어도 무시

	for (const FName& SocketName : ObservationSocketNames)
	{
		FVector SocketLoc = GetMesh()->GetSocketLocation(SocketName);
		FHitResult Hit;
		
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, SocketLoc, ECC_Visibility, Params);

		// 아무것도 안 걸렸거나, 나를 맞췄다면 "보이고 있음"
		if (!bHit || Hit.GetActor() == this)
		{
			return true;
		}
	}

	return false;
}

void ACCD_173::PlayRandomAttackSound()
{
	if (AttackSounds.Num() > 0 && ScreamAudio)
	{
		int32 RandomIndex = FMath::RandRange(0, AttackSounds.Num() - 1);
		
		if (AttackSounds[RandomIndex])
		{
			ScreamAudio->SetSound(AttackSounds[RandomIndex]);
			ScreamAudio->Play();
			
			UE_LOG(LogTemp, Log, TEXT("SCP-173 Attack Sound Index: %d"), RandomIndex);
		}
	}
}

void ACCD_173::StartMoveSound()
{
	if (MoveAudio && MoveSound && !MoveAudio->IsPlaying())
	{
		MoveAudio->SetSound(MoveSound);
		MoveAudio->Play();
	}
}

void ACCD_173::StopMoveSound()
{
	if (MoveAudio && MoveAudio->IsPlaying())
	{
		MoveAudio->Stop();
	}
}