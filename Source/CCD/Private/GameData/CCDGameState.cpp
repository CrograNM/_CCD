
#include "GameData/CCDGameState.h"

#include "Net/UnrealNetwork.h"

void ACCDGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDGameState, bIsCleaningFinished);
}

void ACCDGameState::OnRep_CleaningFinished()
{
}
