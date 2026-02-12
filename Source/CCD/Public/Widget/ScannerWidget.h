
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScannerWidget.generated.h"

UCLASS()
class CCD_API UScannerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Scanner")
	void UpdateDistanceDisplay(float Distance);
};
