
#include "Component/CCD_StatComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Player/CCDCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCCD_StatComponent::UCCD_StatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
	CurrentStamina = MaxStamina;
}

void UCCD_StatComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());
	SetComponentTickEnabled(true);
}

void UCCD_StatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 서버에서만 상태 판단 및 스태미나 계산 수행
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		if (bIsRunning)
		{
			// 달리기 중: 스태미나 소모
			CurrentStamina = FMath::Clamp(CurrentStamina - (StaminaConsumptionRate * DeltaTime), 0.f, MaxStamina);
            
			// 스태미나 고갈 시 달리기 강제 종료
			if (CurrentStamina <= 0.f)
			{
				SetIsRunning(false);
				bIsExhausted = true;
				
				OnRep_IsExhausted();
				
				GetWorld()->GetTimerManager().SetTimer(ExhaustionTimerHandle, this, &UCCD_StatComponent::ResetExhaustion, ExhaustionDelay, false);
			}
		}
		else
		{
			// 걷기 혹은 정지 중: 스태미나 회복
			if (!bIsExhausted && CurrentStamina < MaxStamina)
			{
				CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.f, MaxStamina);
			}
		}
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
	
	// 시야 쿨타임 처리 
	if (OwnerCharacter && OwnerCharacter->HasAuthority() && !bIsEyeClosed)
	{
		if (EyeCooldownTime < EyeCooldownDuration)
		{
			EyeCooldownTime = FMath::Clamp(EyeCooldownTime + DeltaTime, 0.f, EyeCooldownDuration);
			OnEyeCooldownChanged.Broadcast(EyeCooldownTime, EyeCooldownDuration);
		}
		else
		{
			CloseEye();
		}
	}
	
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		CurrentVignetteIntensity = FMath::FInterpTo(CurrentVignetteIntensity, TargetVignetteIntensity, DeltaTime, VignetteInterpSpeed);

		// 카메라 컴포넌트에 적용
		if (UCameraComponent* FPCamera = OwnerCharacter->GetFirstPersonCamera())
		{
			FPCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
			FPCamera->PostProcessSettings.VignetteIntensity = CurrentVignetteIntensity;
		}
	}
}

void UCCD_StatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_StatComponent, bIsRunning);
	DOREPLIFETIME(UCCD_StatComponent, CurrentStamina);
	DOREPLIFETIME(UCCD_StatComponent, bIsExhausted);
	DOREPLIFETIME(UCCD_StatComponent, bIsEyeClosed);
	DOREPLIFETIME(UCCD_StatComponent, EyeCooldownTime);
}

// --- 달리기, 스태미나 ---
void UCCD_StatComponent::SetIsRunning(const bool bNewIsRunning)
{
	if (!OwnerCharacter) return;
	
	if (bNewIsRunning && bIsExhausted) return;
	
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewIsRunning ? RunSpeed : WalkSpeed;
	Server_SetSpeed(bNewIsRunning);
}
void UCCD_StatComponent::Server_SetSpeed_Implementation(const bool bNewIsRunning)
{
	if (bNewIsRunning && bIsExhausted)
	{
		bIsRunning = false;
	}
	else
	{
		bIsRunning = bNewIsRunning;
	}
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewIsRunning ? RunSpeed : WalkSpeed;
}
void UCCD_StatComponent::OnRep_CurrentStamina()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UCCD_StatComponent::OnRep_IsExhausted()
{
	// 로컬 플레이어의 화면에서만 효과가 나타나도록 체크
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		UCameraComponent* FPCamera = OwnerCharacter->GetFirstPersonCamera();
		if (!FPCamera) return;

		TargetVignetteIntensity = bIsExhausted ? 0.8f : 0.0f;
		
		if (bIsExhausted)
		{

			FPCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
			FPCamera->PostProcessSettings.VignetteIntensity = 0.8f;
			
			FPCamera->PostProcessSettings.bOverride_SceneFringeIntensity = true;
			FPCamera->PostProcessSettings.SceneFringeIntensity = 5.0f; 
			
			if (ExhaustedSound && !ExhaustedAudioComp)
			{
				ExhaustedAudioComp = UGameplayStatics::SpawnSoundAttached(ExhaustedSound, OwnerCharacter->GetMesh());
			}
			if (ExhaustedAudioComp) ExhaustedAudioComp->Play();
		}
		else
		{
			FPCamera->PostProcessSettings.VignetteIntensity = 0.0f;
			FPCamera->PostProcessSettings.SceneFringeIntensity = 0.0f;

			if (ExhaustedAudioComp)
			{
				ExhaustedAudioComp->FadeOut(1.0f, 0.0f);
			}
		}
	}
}

void UCCD_StatComponent::ResetExhaustion()
{
	bIsExhausted = false;
	OnRep_IsExhausted();
}

// --- 시야 판정, 쿨타임 ---
void UCCD_StatComponent::CloseEye()
{
	Server_CloseEye();
}
void UCCD_StatComponent::Server_CloseEye_Implementation()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		bIsEyeClosed = true;
		OnRep_IsEyeClosed();
	}, EyeCloseTime, false);
	Multicast_PlayEyeClosedAnimation();
}
void UCCD_StatComponent::OnRep_IsEyeClosed()
{
	if (bIsEyeClosed)
	{
		EyeCooldownTime = 0.f;		// 쿨타임 초기화
		
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			bIsEyeClosed = false;
			OnRep_IsEyeClosed();
		}, EyeOpenTime, false);
	}
}
void UCCD_StatComponent::Multicast_PlayEyeClosedAnimation_Implementation()
{
	// 서버와 모든 클라이언트에서 실행됨
	EyeCooldownTime = 0.f;       // 쿨타임 초기화
	OnEyeClosed.Broadcast();     // 델리게이트 발생
}
void UCCD_StatComponent::OnRep_EyeCooldownTime()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		OnEyeCooldownChanged.Broadcast(EyeCooldownTime, EyeCooldownDuration);
}