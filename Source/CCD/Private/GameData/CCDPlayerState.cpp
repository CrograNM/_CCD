
#include "GameData/CCDPlayerState.h"

#include "Net/UnrealNetwork.h"

void ACCDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDPlayerState, CustomName);
	DOREPLIFETIME(ACCDPlayerState, bIsDead);
	DOREPLIFETIME(ACCDPlayerState, RemainingRespawnTime);
}

void ACCDPlayerState::OnRep_CustomName() const
{
	UE_LOG(LogTemp, Log, TEXT("Player Name Changed : %s"), *CustomName);
}
