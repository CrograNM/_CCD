
#include "GameData/CCDGameState.h"

#include "Net/UnrealNetwork.h"

void ACCDGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDGameState, bIsCleaningFinished);
	DOREPLIFETIME(ACCDGameState, SharedLives);
}

void ACCDGameState::OnRep_CleaningFinished()
{
}

void ACCDGameState::OnRep_SharedLives()
{
	
}
