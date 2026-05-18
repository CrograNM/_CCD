
#include "Actor/EndingTruck.h"

#include "Components/BoxComponent.h"
#include "GameData/CCDGameInstance.h"
#include "Player/CCDCharacter.h"


AEndingTruck::AEndingTruck()
{
}

void AEndingTruck::StartLevelTravel()
{
	if (!HasAuthority()) return;
	
	TArray<AActor*> OverlappingActors;
	WaitingArea->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		if (ACCDCharacter* Character = Cast<ACCDCharacter>(Actor))
		{
			Character->DestroyAllEquipment();
		}
	}
	
	if (UCCDGameInstance* GI = Cast<UCCDGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->LeaveSessionForEnding();
	}
}
