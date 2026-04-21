
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpectatorWidget.generated.h"

class ACCDCharacter;

UCLASS()
class CCD_API USpectatorWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Spectate")
	void UpdateSpectatorInfo(ACCDCharacter* TargetCharacter);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Spectate")
	void StartRespawnCountdown(float RespawnTime);
};
