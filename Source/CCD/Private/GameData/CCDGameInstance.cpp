
#include "GameData/CCDGameInstance.h"
#include "GameData/CCDSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCCDGameInstance::Init()
{
	Super::Init();
	UserProfileName = GetSavedName();
}

void UCCDGameInstance::SaveCustomName(FString NewName)
{
	UserProfileName = NewName;
	if (UCCDSaveGame* SaveInstance = Cast<UCCDSaveGame>(UGameplayStatics::CreateSaveGameObject(UCCDSaveGame::StaticClass())))
	{
		SaveInstance->SavedPlayerName = NewName;
		UGameplayStatics::SaveGameToSlot(SaveInstance, SaveSlotName, 0);
	}
}

FString UCCDGameInstance::GetSavedName() const
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		if (UCCDSaveGame* LoadedGame = Cast<UCCDSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
		{
			return LoadedGame->SavedPlayerName;
		}
	}
	return TEXT("None");
}
