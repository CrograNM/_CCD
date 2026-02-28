#pragma once

#include "CoreMinimal.h"
#include "CCD_096_States.generated.h"

UENUM(BlueprintType)
enum class E096State : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Panic UMETA(DisplayName = "Panic"),
	Enraged UMETA(DisplayName = "Enraged")
};