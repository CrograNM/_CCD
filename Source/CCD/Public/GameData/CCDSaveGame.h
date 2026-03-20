
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CCDSaveGame.generated.h"

UCLASS()
class CCD_API UCCDSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FString SavedPlayerName;
};
