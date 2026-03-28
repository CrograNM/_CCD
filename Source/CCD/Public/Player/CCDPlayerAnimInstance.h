
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float ForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RightSpeed;

	// CCD 캐릭터 참조용
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<ACCDCharacter> OwnerCharacter;
};
