
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

UENUM(BlueprintType)
enum class ECCD_EquipmentState : uint8
{
	EES_Hands   UMETA(DisplayName = "Hands"),   // 맨손 (Physics Handle)
	EES_Scanner UMETA(DisplayName = "Scanner"), // 탐지장치
	EES_Mop     UMETA(DisplayName = "Mop")		// 대걸레
};

#include "CCDCharacter.h"
#include "CCD_EquipmentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UCCD_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCD_EquipmentComponent();

protected:
	virtual void BeginPlay() override;

public:	

		
};
