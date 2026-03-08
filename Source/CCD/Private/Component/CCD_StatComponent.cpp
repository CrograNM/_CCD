
#include "Component/CCD_StatComponent.h"

#include "CCDCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	if (OwnerCharacter->HasAuthority())
	{
		if (bIsRunning)
		{
			// 달리기 중: 스태미나 소모
			CurrentStamina = FMath::Clamp(CurrentStamina - (StaminaConsumptionRate * DeltaTime), 0.f, MaxStamina);
            
			// 스태미나 고갈 시 달리기 강제 종료
			if (CurrentStamina <= 0.f)
			{
				SetIsRunning(false);
			}
		}
		else
		{
			// 걷기 혹은 정지 중: 스태미나 회복
			if (CurrentStamina < MaxStamina)
			{
				CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.f, MaxStamina);
			}
		}
	}
	if (OwnerCharacter->IsLocallyControlled())
		UE_LOG(LogTemp, Log, TEXT("[Local] Stamina: %f"), CurrentStamina);
}

void UCCD_StatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_StatComponent, bIsRunning);
	DOREPLIFETIME(UCCD_StatComponent, CurrentStamina);
}

// 달리기 상태 변경
void UCCD_StatComponent::Server_SetSpeed_Implementation(const bool bNewIsRunning)
{
	bIsRunning = bNewIsRunning;
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewIsRunning ? RunSpeed : WalkSpeed;
}
void UCCD_StatComponent::SetIsRunning(const bool bNewIsRunning)
{
	if (!OwnerCharacter) return;
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewIsRunning ? RunSpeed : WalkSpeed;
	Server_SetSpeed(bNewIsRunning);
}