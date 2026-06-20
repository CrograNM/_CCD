// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_939.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CCDCharacter.h"

class UBlackboardComponent;
// Sets default values
ACCD_939::ACCD_939()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	
	// 1. 머리 콜리전 생성 및 부착
	HeadCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadCollision"));
	HeadCollision->SetupAttachment(GetMesh(), TEXT("head")); // 실제 939 스켈레톤의 머리 뼈 이름을 적으세요
	
	// 2. 몸통 콜리전 생성 및 부착
	BodyCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyCollision"));
	BodyCollision->SetupAttachment(GetMesh(), TEXT("chest")); // 척추 뼈 이름

	// 주의: 이 추가 콜리전들이 지형지물(벽, 바닥)에 걸려 이동을 방해하면 안 됩니다!
	HeadCollision->SetCollisionProfileName(TEXT("OverlapOnlyPawn")); // 예시: Pawn하고만 상호작용
	BodyCollision->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
}

// Called when the game starts or when spawned
void ACCD_939::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCD_939::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCD_939::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACCD_939::SetMovementState(bool bIsChasing)
{
	GetCharacterMovement()->MaxWalkSpeed = bIsChasing ? ChaseSpeed : PatrolSpeed;
}

void ACCD_939::ExecuteAttack()
{
	if (!HasAuthority()) return;

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
			
			if (TargetActor && TargetActor->IsA(ACCDCharacter::StaticClass()))
			{
				float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
				
				if (Dist <= 500.0f)
				{
					UGameplayStatics::ApplyDamage(
						TargetActor, 
						1.0f, 
						AIC, 
						this, 
						UDamageType::StaticClass()
					);
					Multicast_PlayAttackSound();
				}
			}
		}
	}
}

void ACCD_939::Multicast_PlayStateSound_Implementation(USoundBase* SoundToPlay, bool bAtLocation)
{
	if (SoundToPlay)
	{
		if (bAtLocation)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this, 
				SoundToPlay, 
				GetActorLocation(), 
				1.0f,               
				1.0f                
			);
		}
		else
		{
			UGameplayStatics::PlaySound2D(this, SoundToPlay);
		}
	}
}

void ACCD_939::Multicast_SetFreezeVisual_Implementation(bool bFreeze)
{
	USkeletalMeshComponent* TargetMesh = GetMesh();
	if (!TargetMesh) return;
	
	TargetMesh->bNoSkeletonUpdate = bFreeze;
	TargetMesh->SetComponentTickEnabled(!bFreeze);

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
				Mat->SetScalarParameterValue(TEXT("FreezeAmount"), 0.2f);
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

void ACCD_939::Multicast_PlayAttackMontage_Implementation(UAnimMontage* MontageToPlay)
{
	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
}

void ACCD_939::Multicast_PlayAttackSound_Implementation()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			AttackSound, 
			GetActorLocation(), 
			1.0f,               
			1.0f                
		);
	}
}
