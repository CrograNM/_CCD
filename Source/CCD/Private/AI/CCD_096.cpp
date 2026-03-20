// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_096.h"

#include "AIController.h"
#include "Player/CCDCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"

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
	
	ScreamAudio_Secondary = CreateDefaultSubobject<UAudioComponent>(TEXT("ScreamAudio_Secondary"));
	ScreamAudio_Secondary->SetupAttachment(GetRootComponent());
	ScreamAudio_Secondary->bAutoActivate = false;
	
	bReplicates = true;
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

void ACCD_096::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCD_096, CurrentState);
}

void ACCD_096::MarkPlayer(AActor* Player)
{
	if (!HasAuthority() || !Player) return;
	
	ACCDCharacter* TargetChar = Cast<ACCDCharacter>(Player);
	if (TargetList.Contains(Player) || (TargetChar && TargetChar->IsDead())) return;
	
	TargetList.Add(Player);
	UE_LOG(LogTemp, Log, TEXT("Player Marked: %s (Total: %d)"), *Player->GetName(), TargetList.Num());
	
	if (CurrentState == E096State::Idle)
	{
		TriggerPanic(Player);
	}
}

void ACCD_096::TriggerPanic(AActor* Player)
{
	if (!HasAuthority() || CurrentState != E096State::Idle) return;

	SetState(E096State::Panic);
    
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsObject(TEXT("TargetActor"), Player);
			AIC->StopMovement();
		}
	}
}

AActor* ACCD_096::GetNextTarget()
{
	TargetList.RemoveAll([](const TObjectPtr<AActor>& A) {
		ACCDCharacter* C = Cast<ACCDCharacter>(A);
		return !A || (C && C->IsDead());
	});

	return (TargetList.Num() > 0) ? TargetList[0] : nullptr;
}

void ACCD_096::SetState(E096State NewState)
{
	if (!HasAuthority()) return;
	CurrentState = NewState;
	OnRep_CurrentState();

	if (AAIController* AIC = Cast<AAIController>(GetController()))
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			BB->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentState));
}

void ACCD_096::OnRep_CurrentState()
{
	switch (CurrentState)
	{
	case E096State::Idle:
		StopScreamSound();
		break;
	case E096State::Panic:
		PlayPanicSound();
		// 여기서 "부들부들 떠는" 애니메이션 몽타주 재생
		break;
	case E096State::Enraged:
		PlayChaseSound();
		// 여기서 "미친 듯이 달려오는" 애니메이션으로 전환
		break;
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

void ACCD_096::PlayPanicSound()
{
	if (ScreamAudio && PanicSound)
	{
		ScreamAudio->SetSound(PanicSound);
		ScreamAudio->Play();
	}
	
	if (ScreamAudio_Secondary && PanicSound_2)
	{
		ScreamAudio_Secondary->SetSound(PanicSound_2);
		ScreamAudio_Secondary->Play();
	}
}

void ACCD_096::PlayChaseSound()
{
	if (ScreamAudio && ChaseSound)
	{
		if (ScreamAudio->Sound == ChaseSound && ScreamAudio->IsPlaying())
		{
			return; 
		}

		ScreamAudio->SetSound(ChaseSound);
		ScreamAudio->Play();
	}
}

void ACCD_096::StopScreamSound()
{
	if (ScreamAudio && ScreamAudio->IsPlaying()) ScreamAudio->Stop();
	if (ScreamAudio_Secondary && ScreamAudio_Secondary->IsPlaying()) ScreamAudio_Secondary->Stop();
}

void ACCD_096::Multicast_PlayKillSound_Implementation()
{
	if (KillSound)
	{
		StopScreamSound(); 
        

		UGameplayStatics::PlaySoundAtLocation(
			this, 
			KillSound, 
			GetActorLocation(), 
			1.0f, 
			1.0f 
		);
        
		UE_LOG(LogTemp, Log, TEXT("096 Kill Sound Played as One-Shot"));
	}
}