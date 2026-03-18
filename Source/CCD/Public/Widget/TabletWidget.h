
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabletWidget.generated.h"

UCLASS()
class CCD_API UTabletWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Tablet")
	void UpdateTablet();
};
