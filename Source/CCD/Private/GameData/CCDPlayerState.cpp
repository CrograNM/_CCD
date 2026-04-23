
#include "GameData/CCDPlayerState.h"

#include "Net/UnrealNetwork.h"

void ACCDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDPlayerState, CustomName);
	DOREPLIFETIME(ACCDPlayerState, SteamName);
	DOREPLIFETIME(ACCDPlayerState, bIsDead);
	DOREPLIFETIME(ACCDPlayerState, RespawnStartTime);
	DOREPLIFETIME(ACCDPlayerState, RespawnEndTime);
}

void ACCDPlayerState::OnRep_CustomName() const
{
	UE_LOG(LogTemp, Log, TEXT("Player Name Changed : %s"), *CustomName);
}
