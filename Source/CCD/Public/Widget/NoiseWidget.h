
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoiseWidget.generated.h"

UCLASS()
class CCD_API UNoiseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Noise")
	void UpdateNoiseDisplay(float NoiseLevel);
};
