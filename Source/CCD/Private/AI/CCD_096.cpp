// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_096.h"

#include "AIController.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

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
			BB->SetValueAsEnum(TEXT("AIState"), 1); // Panic 상태로 변경
			BB->SetValueAsObject(TEXT("TargetActor"), Player);
			
			// 즉시 이동 중지
			AIC->StopMovement();
		}
	}
}

bool ACCD_096::IsTriggered() const
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			return BB->GetValueAsEnum(TEXT("AIState")) != 0;
		}
	}
	return false;
}