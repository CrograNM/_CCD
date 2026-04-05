
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Component/CCD_EquipmentComponent.h"
#include "CCDPlayerAnimInstance.generated.h"

class ACCDCharacter;

UCLASS()
class CCD_API UCCDPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<ACCDCharacter> OwnerCharacter; // CCD 캐릭터 참조용
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float ForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RightSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	ECCD_EquipmentState EquipmentState;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emote")
	bool bIsEmoting = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	bool bIsGrabbing = false;
};
