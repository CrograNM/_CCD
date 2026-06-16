// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_173.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Player/CCDCharacter.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	
	bReplicates = true;
}

void ACCD_173::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(TEXT("CanMove"), false);
				BB->SetValueAsBool(TEXT("HasSpottedPlayer"), false);
			}
		}
	}
}

void ACCD_173::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetNetMode() != NM_DedicatedServer)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			float Distance = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());

			if (Distance <= CriticalThreshold)
			{
				if (!bCriticalSoundPlayed && CriticalHorrorSound)
				{
					UGameplayStatics::PlaySound2D(this, CriticalHorrorSound);
					bCriticalSoundPlayed = true;
					bNearSoundPlayed = true;
				}
			}
			else if (Distance <= NearThreshold)
			{
				if (!bNearSoundPlayed && NearHorrorSound)
				{
					UGameplayStatics::PlaySound2D(this, NearHorrorSound);
					bNearSoundPlayed = true;
				}
			}
			else if (Distance > NearThreshold + ResetDistanceMargin)
			{
				bNearSoundPlayed = false;
				bCriticalSoundPlayed = false;
			}
		}
	}
}

void ACCD_173::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ACCD_173::IsObserved()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		ACCDCharacter* Character = Cast<ACCDCharacter>(PC->GetPawn());

		if (!Character || Character->IsDead() || !Character->GetIsObserveActivated()) continue;

		APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
		if (!CameraManager) continue;

		FVector CameraLoc = CameraManager->GetCameraLocation();
		FVector CameraForward = CameraManager->GetCameraRotation().Vector();

		// 시야각(FOV) 체크
		FVector ToMe = GetActorLocation() - CameraLoc;
		ToMe.Normalize();
		float Dot = FVector::DotProduct(CameraForward, ToMe);

		if (Dot < VisibilityThreshold) continue; 

		// 가려짐 체크 (Line Trace)
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(Character);

		for (const FName& SocketName : ObservationSocketNames)
		{
			FVector SocketLoc = GetMesh()->GetSocketLocation(SocketName);
			FHitResult Hit;
			
			if (!GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, SocketLoc, ECC_Visibility, Params) || Hit.GetActor() == this)
			{
				return true; 
			}
		}
	}
	return false;
}

void ACCD_173::PlayRandomAttackSound()
{
	if (HasAuthority() && AttackSounds.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AttackSounds.Num() - 1);
		Multicast_PlayAttackSound(RandomIndex);
	}
}

// 이동 사운드 제어
void ACCD_173::StartMoveSound()
{
	if (HasAuthority()) Multicast_StartMoveSound();
}

void ACCD_173::StopMoveSound()
{
	if (HasAuthority()) Multicast_StopMoveSound();
}



void ACCD_173::Multicast_PlayAttackSound_Implementation(int32 SoundIndex)
{
	if (AttackSounds.IsValidIndex(SoundIndex) && ScreamAudio && !ScreamAudio->IsPlaying())
	{
		ScreamAudio->SetSound(AttackSounds[SoundIndex]);
		ScreamAudio->Play();
	}
}

void ACCD_173::Multicast_StartMoveSound_Implementation()
{
	if (MoveAudio && MoveSound && !MoveAudio->IsPlaying())
	{
		MoveAudio->SetSound(MoveSound);
		MoveAudio->Play();
	}
}

void ACCD_173::Multicast_StopMoveSound_Implementation()
{
	if (MoveAudio && MoveAudio->IsPlaying())
	{
		MoveAudio->Stop();
	}
}

void ACCD_173::SetMovementInstant(bool bInstant)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (bInstant)
		{
			MoveComp->MaxWalkSpeed = 4000.0f; 
			MoveComp->MaxAcceleration = 10000.0f;
			MoveComp->bRequestedMoveUseAcceleration = false;
		}
		else
		{
			MoveComp->MaxWalkSpeed = 0.0f;
			MoveComp->Velocity = FVector::ZeroVector;
		}
	}
}

void ACCD_173::Multicast_SetFreezeVisual_Implementation(bool bFreeze)
{
	USkeletalMeshComponent* TargetMesh = GetMesh();
	if (!TargetMesh) return;

	if (bFreeze)
	{
		if (DynamicMaterials.Num() == 0)
		{
			int32 MaterialCount = TargetMesh->GetNumMaterials();
			for (int32 MatIndex = 0; MatIndex < MaterialCount; ++MatIndex)
			{
				UMaterialInstanceDynamic* DynamicMat = TargetMesh->CreateAndSetMaterialInstanceDynamic(MatIndex);
				if (DynamicMat)
				{
					DynamicMaterials.Add(DynamicMat);
				}
			}
		}

		for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
		{
			if (Mat)
			{
				Mat->SetScalarParameterValue(TEXT("FreezeAmount"), 1.0f);
			}
		}
	}
	else
	{
		for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
		{
			if (Mat)
			{
				Mat->SetScalarParameterValue(TEXT("FreezeAmount"), 0.0f);
			}
		}
	}
}
