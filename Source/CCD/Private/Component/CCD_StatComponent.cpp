
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
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());
}

void UCCD_StatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.f, MaxStamina);
	}
}

void UCCD_StatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_StatComponent, bIsRunning);
}

// 달리기 상태 변경
void UCCD_StatComponent::Server_SetSpeed_Implementation(const bool bNewIsRunning)
{
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewIsRunning ? RunSpeed : WalkSpeed;
}
void UCCD_StatComponent::SetIsRunning(const bool bNewIsRunning)
{
	if (!OwnerCharacter) return;
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewIsRunning ? RunSpeed : WalkSpeed;
	Server_SetSpeed(bNewIsRunning);
}
void UCCD_StatComponent::OnRep_IsRunning()
{
}