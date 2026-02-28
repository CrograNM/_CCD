// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_096.h"

#include "AIController.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CCD_096_States.h"

// Sets default values
ACCD_096::ACCD_096()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트 생성 및 배치
	FaceTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("FaceTrigger"));
	FaceTrigger->SetupAttachment(GetMesh(), TEXT("head")); // 머리에 부착

	ScreamAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ScreamAudio"));
	ScreamAudio->SetupAttachment(GetRootComponent());
	ScreamAudio->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ACCD_096::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCD_096::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCD_096::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACCD_096::TriggerPanic(AActor* Player)
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			// 1. 상태를 Panic으로 변경
			BB->SetValueAsEnum(TEXT("AIState"), (uint8)E096State::Panic);
			BB->SetValueAsObject(TEXT("TargetActor"), Player);

			// 2. 비명 재생
			if (PanicSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, PanicSound, GetActorLocation());
			}
		}
	}
}

bool ACCD_096::IsTriggered() const
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			// AIState가 Idle이 아니면 이미 트리거된 것으로 간주
			return BB->GetValueAsEnum(TEXT("AIState")) != (uint8)E096State::Idle;
		}
	}
	return false;
}