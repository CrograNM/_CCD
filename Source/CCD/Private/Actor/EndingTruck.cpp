
#include "Actor/EndingTruck.h"

#include "Components/BoxComponent.h"
#include "GameData/CCDGameInstance.h"
#include "GameData/CCDGameMode.h"
#include "Player/CCDCharacter.h"
#include "Player/CCDPlayerControllerBase.h"


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
	
	NextLevelPath = "/Game/Maps/Ending";
	// 월드의 모든 플레이어 컨트롤러를 순회하며 개별 이동 RPC를 날립니다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACCDPlayerControllerBase* PC = Cast<ACCDPlayerControllerBase>(It->Get()))
		{
			PC->Client_MoveToEndingLocal(NextLevelPath);
		}
	}
}
